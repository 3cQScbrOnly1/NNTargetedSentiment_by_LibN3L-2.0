#ifndef SRC_Model_params_H_
#define SRC_Model_params_H_

#include "HyperParams.h"

class ModelParams{
public:
	LookupTable words1;
	LookupTable words2;

	UniParams represent_transform_layer;
	AttRecursiveGatedParams arg_layer;
	UniParams olayer_linear;
public:
	Alphabet wordAlpha;
	Alphabet labelAlpha;
	Alphabet featAlpha;
	Alphabet charAlpha;

	SoftMaxLoss loss;

public:
	bool initial(HyperParams& hyper_params){
		if (words1.nVSize <= 0 || words2.nVSize <= 0 || labelAlpha.size() <= 0)
			return false;
		hyper_params.wordDim1 = words1.nDim;
		hyper_params.wordDim2 = words2.nDim;
		hyper_params.wordDim = hyper_params.wordDim1 + hyper_params.wordDim2;
		hyper_params.labelSize = labelAlpha.size();
		hyper_params.wordWindow = hyper_params.wordContext * 2 + 1;
		hyper_params.windowOutputSize = hyper_params.wordDim * hyper_params.wordWindow;
		hyper_params.representInputSize = hyper_params.windowOutputSize * 4;
		hyper_params.inputSize = hyper_params.hiddenSize + hyper_params.representInputSize * 3;

		represent_transform_layer.initial(hyper_params.hiddenSize, hyper_params.representInputSize);

		arg_layer.initial(hyper_params.hiddenSize, hyper_params.hiddenSize);

		olayer_linear.initial(hyper_params.labelSize, hyper_params.inputSize, false);
		return true;
	}

	void exportModelParams(ModelUpdate& ada) {
		words1.exportAdaParams(ada);
		words2.exportAdaParams(ada);
		represent_transform_layer.exportAdaParams(ada);
		arg_layer.exportAdaParams(ada);
		olayer_linear.exportAdaParams(ada);
	}

	void exportCheckGradParams(CheckGrad& checkgrad) {
		checkgrad.add(&(represent_transform_layer.W), "represent_transform_layer.W");
		checkgrad.add(&(arg_layer._update_left_param.W1), "arg_layer._update_left.W1");
		checkgrad.add(&(arg_layer._update_left_param.W2), "arg_layer._update_left.W2");
		checkgrad.add(&(arg_layer._update_right_param.W1), "arg_layer._update_right.W1");
		checkgrad.add(&(arg_layer._update_right_param.W2), "arg_layer._update_right.W2");
		checkgrad.add(&(arg_layer._reset_left_param.W1), "arg_layer._reset_left.W1");
		checkgrad.add(&(arg_layer._reset_left_param.W2), "arg_layer._reset_left.W2");
		checkgrad.add(&(arg_layer._reset_right_param.W1), "arg_layer._reset_right.W1");
		checkgrad.add(&(arg_layer._reset_right_param.W2), "arg_layer._reset_right.W2");
		checkgrad.add(&(arg_layer._update_tilde_param.W1), "arg_layer._update_tilde.W1");
		checkgrad.add(&(arg_layer._update_tilde_param.W2), "arg_layer._update_tilde.W2");
		checkgrad.add(&(arg_layer._recursive_tilde_param.W1), "arg_layer._recursive_tilde.W1");
		checkgrad.add(&(arg_layer._recursive_tilde_param.W2), "arg_layer._recursive_tilde.W2");
		checkgrad.add(&(olayer_linear.W), "olayer_linear.W");
	}

	void saveModel(){
	}

	void loadModel(const string& infile) {
	}
};

#endif
