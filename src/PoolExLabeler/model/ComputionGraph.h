#ifndef SRC_ComputionGraph_H_
#define SRC_ComputionGraph_H_

#include "ModelParams.h"

struct ComputionGraph :Graph{
public:
	const static int max_sentence_length = 256;
private:
	int windowOutputSize;

public:
	vector<LookupNode> _word_inputs1;
	vector<LookupNode> _word_inputs2;
	vector<ConcatNode> _word_concats;
	WindowBuilder _word_window;

	MaxPoolNode _left_max_pooling;
	MinPoolNode _left_min_pooling;
	AvgPoolNode _left_avg_pooling;
	StdPoolNode _left_std_pooling;
	ConcatNode _left_pooling_concat;

	MaxPoolNode _target_max_pooling;
	MinPoolNode _target_min_pooling;
	AvgPoolNode _target_avg_pooling;
	StdPoolNode _target_std_pooling;
	ConcatNode _target_pooling_concat;

	MaxPoolNode _right_max_pooling;
	MinPoolNode _right_min_pooling;
	AvgPoolNode _right_avg_pooling;
	StdPoolNode _right_std_pooling;
	ConcatNode _right_pooling_concat;
	Node _pooling_concat_zero;
	ConcatNode _pooling_concat;
	vector<UniNode> _represent_transforms;
	AttRecursiveGatedBuilder _target_attention;

	ConcatNode _concat;
	LinearNode _output;
public:
	ComputionGraph():Graph(){
	}

	~ComputionGraph(){
		clear();
	}
public:
	inline void createNodes(int sent_length) {
		_word_inputs1.resize(sent_length);
		_word_inputs2.resize(sent_length);
		_word_concats.resize(sent_length);
		_word_window.resize(sent_length);

		_left_max_pooling.setParam(sent_length);
		_left_min_pooling.setParam(sent_length);
		_left_avg_pooling.setParam(sent_length);
		_left_std_pooling.setParam(sent_length);

		_right_max_pooling.setParam(sent_length);
		_right_min_pooling.setParam(sent_length);
		_right_avg_pooling.setParam(sent_length);
		_right_std_pooling.setParam(sent_length);

		_target_max_pooling.setParam(sent_length);
		_target_min_pooling.setParam(sent_length);
		_target_avg_pooling.setParam(sent_length);
		_target_std_pooling.setParam(sent_length);

		_represent_transforms.resize(3);
	}

	inline void clear(){
		_word_inputs1.clear();
		_word_inputs2.clear();
		_word_concats.clear();
		_word_window.clear();
	}

public:
	inline void initial(ModelParams& model_params, HyperParams& hyper_params, AlignedMemoryPool *mem = NULL) {
		int maxsize = _word_inputs1.size();
		for (int idx = 0; idx < maxsize; idx++)
		{
			_word_inputs1[idx].setParam(&model_params.words1);
			_word_inputs1[idx].init(hyper_params.wordDim1, hyper_params.dropOut, mem);
			_word_inputs2[idx].setParam(&model_params.words2);
			_word_inputs2[idx].init(hyper_params.wordDim2, hyper_params.dropOut, mem);
			_word_concats[idx].init(hyper_params.wordDim, -1, mem);
		}
		windowOutputSize = hyper_params.windowOutputSize;
		_word_window.init(hyper_params.wordDim, hyper_params.wordContext, mem);
		_pooling_concat_zero.init(windowOutputSize * 4, -1, mem);


		_left_max_pooling.init(windowOutputSize, -1, mem);
		_left_min_pooling.init(windowOutputSize, -1, mem);
		_left_avg_pooling.init(windowOutputSize, -1, mem);
		_left_std_pooling.init(windowOutputSize, -1, mem);

		_left_pooling_concat.init(windowOutputSize * 4, -1, mem);


		_right_max_pooling.init(windowOutputSize, -1, mem);
		_right_min_pooling.init(windowOutputSize, -1, mem);
		_right_avg_pooling.init(windowOutputSize, -1, mem);
		_right_std_pooling.init(windowOutputSize, -1, mem);

		_right_pooling_concat.init(windowOutputSize * 4, -1, mem);

		_target_max_pooling.init(windowOutputSize, -1, mem);
		_target_min_pooling.init(windowOutputSize, -1, mem);
		_target_avg_pooling.init(windowOutputSize, -1, mem);
		_target_std_pooling.init(windowOutputSize, -1, mem);

		_target_pooling_concat.init(windowOutputSize * 4, -1, mem);

		_pooling_concat.init(windowOutputSize * 4 * 3, -1, mem);

		for (int idx = 0; idx < 3; idx++)
		{
			_represent_transforms[idx].setParam(&model_params.represent_transform_layer);
			_represent_transforms[idx].init(hyper_params.hiddenSize, hyper_params.dropOut, mem);
		}
		_target_attention.init(&model_params.arg_layer, hyper_params.dropOut, mem);
		_concat.init(hyper_params.windowOutputSize * 4 * 3 + hyper_params.hiddenSize, -1, mem);
		_output.setParam(&model_params.olayer_linear);
		_output.init(hyper_params.labelSize, -1, mem);
	}

public:
	inline void forward(const vector<Feature>& features, int left, int right, bool bTrain = false){
		clearValue(bTrain);
		int seq_size = features.size();
		for (int idx = 0; idx < seq_size; idx++) {
			const Feature& feature = features[idx];
			_word_inputs1[idx].forward(this, feature.words[0]);
			_word_inputs2[idx].forward(this, feature.words[0]);
			_word_concats[idx].forward(this, &_word_inputs1[idx], &_word_inputs2[idx]);
		}
		_word_window.forward(this, getPNodes(_word_concats, seq_size));
		vector<PNode> three_part_poolings;
		if (left != 0)
		{
			_left_max_pooling.forward(this, getPNodes(_word_window._outputs, left));
			_left_min_pooling.forward(this, getPNodes(_word_window._outputs, left));
			_left_avg_pooling.forward(this, getPNodes(_word_window._outputs, left));
			_left_std_pooling.forward(this, getPNodes(_word_window._outputs, left));
			_left_pooling_concat.forward(this, &_left_max_pooling, &_left_min_pooling, &_left_avg_pooling, &_left_std_pooling);
			three_part_poolings.push_back(&_left_pooling_concat);
		} else
			three_part_poolings.push_back(&_pooling_concat_zero);

		if (right != seq_size - 1)
		{
			_right_max_pooling.forward(this, getPNodes(_word_window._outputs, right + 1, seq_size - right - 1));
			_right_min_pooling.forward(this, getPNodes(_word_window._outputs, right + 1, seq_size - right - 1));
			_right_avg_pooling.forward(this, getPNodes(_word_window._outputs, right + 1, seq_size - right - 1));
			_right_std_pooling.forward(this, getPNodes(_word_window._outputs, right + 1, seq_size - right - 1));
			_right_pooling_concat.forward(this, &_right_max_pooling, &_right_min_pooling, &_right_avg_pooling, &_right_std_pooling);
			three_part_poolings.push_back(&_right_pooling_concat);
		} else
			three_part_poolings.push_back(&_pooling_concat_zero);

		if (right != 0)
		{
			_target_max_pooling.forward(this, getPNodes(_word_window._outputs, left, right - left + 1));
			_target_min_pooling.forward(this, getPNodes(_word_window._outputs, left, right - left + 1));
			_target_avg_pooling.forward(this, getPNodes(_word_window._outputs, left, right - left + 1));
			_target_std_pooling.forward(this, getPNodes(_word_window._outputs, left, right - left + 1));
			_target_pooling_concat.forward(this, &_target_max_pooling, &_target_min_pooling, &_target_avg_pooling, &_target_std_pooling);
			three_part_poolings.push_back(&_target_pooling_concat);
		} else
			three_part_poolings.push_back(&_pooling_concat_zero);


		_pooling_concat.forward(this, three_part_poolings);
		for (int idx = 0; idx < 3; idx++)
			_represent_transforms[idx].forward(this, three_part_poolings[idx]);
		_target_attention.forward(this, &_represent_transforms[0], &_represent_transforms[1], &_represent_transforms[2]);
		_concat.forward(this, &_pooling_concat, &_target_attention._output);
		_output.forward(this, &_concat);

	}
};

#endif