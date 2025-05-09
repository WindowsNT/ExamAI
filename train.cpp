#include "pch.h"

#pragma once
#include <DirectMLX.h>
#include <array>


inline DML_TENSOR_DESC MakeTensorDesc(
	DML_TENSOR_DATA_TYPE dataType,
	const std::vector<uint32_t>& shape
) {
	static thread_local std::vector<uint32_t> sizes, strides;

	sizes = shape;
	strides.resize(shape.size());

	// Compute tightly packed strides
	uint32_t stride = 1;
	for (int i = (int)shape.size() - 1; i >= 0; --i) {
		strides[i] = stride;
		stride *= shape[i];
	}

	DML_BUFFER_TENSOR_DESC bufferDesc = {};
	bufferDesc.DataType = dataType;
	bufferDesc.DimensionCount = (uint32_t)shape.size();
	bufferDesc.Sizes = sizes.data();
	bufferDesc.Strides = strides.data();
	bufferDesc.TotalTensorSizeInBytes =
		DMLCalcBufferTensorSize(dataType, bufferDesc.DimensionCount, sizes.data(), strides.data());

	static thread_local DML_TENSOR_DESC tensorDesc = {};
	tensorDesc.Type = DML_TENSOR_TYPE_BUFFER;
	tensorDesc.Desc = &bufferDesc;
	return tensorDesc;
}


namespace dml2
{
	using namespace dml;


	inline dml::Expression MatMul(
		dml::Expression a, // [M, K]
		dml::Expression b, // [M, N]
		bool transposeA = false,
		bool transposeB = false)
	{
		assert(a.Impl()->GetGraphBuilder() == b.Impl()->GetGraphBuilder());
		auto* builder = a.Impl()->GetGraphBuilder();

		auto aDesc = a.Impl()->GetOutputDesc();
		auto bDesc = b.Impl()->GetOutputDesc();

		DML_GEMM_OPERATOR_DESC gemmDesc = {};
		gemmDesc.ATensor = aDesc.AsPtr<DML_TENSOR_DESC>();
		gemmDesc.BTensor = bDesc.AsPtr<DML_TENSOR_DESC>();
		gemmDesc.OutputTensor = nullptr; // We'll set this below
		gemmDesc.TransA = transposeA ? DML_MATRIX_TRANSFORM_TRANSPOSE : DML_MATRIX_TRANSFORM_NONE;
		gemmDesc.TransB = transposeB ? DML_MATRIX_TRANSFORM_TRANSPOSE : DML_MATRIX_TRANSFORM_NONE;
		gemmDesc.Alpha = 1.0f;
		gemmDesc.Beta = 0.0f;
		gemmDesc.CTensor = nullptr;

		// Output shape: [K, N] where K = input channels, N = output channels
		uint32_t K = transposeA ? aDesc.sizes[1] : aDesc.sizes[0];
		uint32_t N = transposeB ? bDesc.sizes[0] : bDesc.sizes[1];

		dml::TensorDesc outputDesc(
			DML_TENSOR_DATA_TYPE_FLOAT32,
			dml::TensorDimensions{ K, N },
			builder->GetTensorPolicy());

		gemmDesc.OutputTensor = outputDesc.AsPtr<DML_TENSOR_DESC>();

		dml::detail::NodeOutput* const inputs[] = { a.Impl(), b.Impl() };
		auto node = builder->CreateOperatorNode(DML_OPERATOR_GEMM, &gemmDesc, inputs);
		auto* output = builder->CreateNodeOutput(node, 0, std::move(outputDesc));

		return dml::Expression(output);
	}

}

CComPtr<IWICImagingFactory> factory;
bool LoadGrayscaleImageWIC(const std::wstring& filename, int expectedWidth, int expectedHeight, float* outPixels)
{
	CComPtr<IWICBitmapDecoder> decoder;
	CComPtr<IWICBitmapFrameDecode> frame;
	CComPtr<IWICFormatConverter> converter;

	HRESULT hr = S_OK;
	if (factory.p == 0)
		hr = factory.CoCreateInstance(CLSID_WICImagingFactory);
	if (FAILED(hr)) return false;

	hr = factory->CreateDecoderFromFilename(
		filename.c_str(),
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&decoder);
	if (FAILED(hr)) return false;

	hr = decoder->GetFrame(0, &frame);
	if (FAILED(hr)) return false;

	hr = factory->CreateFormatConverter(&converter);
	if (FAILED(hr)) return false;

	hr = converter->Initialize(
		frame,
		GUID_WICPixelFormat8bppGray, // Convert to 8-bit grayscale
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeCustom);
	if (FAILED(hr)) return false;

	UINT w = 0, h = 0;
	converter->GetSize(&w, &h);
	if ((int)w != expectedWidth || (int)h != expectedHeight)
		return false;

	std::vector<BYTE> buffer(w * h);
	hr = converter->CopyPixels(nullptr, w, w * h, buffer.data());
	if (FAILED(hr)) return false;

	for (unsigned int i = 0; i < w * h; ++i)
	{
		outPixels[i] = buffer[i] / 255.0f;
	}

	return true;
}


std::vector<std::wstring> LoadPngFilesFromDirectory(const std::wstring& folderPath)
{
	std::vector<std::wstring> files;
	std::wstring searchPath = folderPath + L"\\*.png";

	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE)
		return files;

	do {
		if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			files.push_back(folderPath + L"\\" + findData.cFileName);
		}
	} while (FindNextFile(hFind, &findData));

	FindClose(hFind);
	return files;
}


MLOP GetBPDML(ML& ml, MLOP& op1, unsigned int B, unsigned int wi, unsigned int he)
{
	MLOP op2(&ml);
	const int IT_INPUT = 0;
	const int IT_LABELS = 1;
	const int IT_INWEGHTS = 2;
	const int IT_INFROM = 3;
//	const int IT_ACTS = 4;
	const int IT_GRADOUT = 5;

	// === Inputs ===
	op2.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { B, 4, 1, 1 } });                      // Item(0): Logits
	op2.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { B, 4, 1, 1 } });                      // Item(1): Labels
//	op2.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 4, 64, 1, 1 } });                     // Item(2): Trainable weights (conv3)
	op2.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 4, 64, 1, 1 } },0,0,BINDING_MODE::BIND_IN,op1.Item(3).buffer->b);                     // Item(2): Trainable weights (conv3)
	op2.AddInput(op1.Item(8).expr.GetOutputDesc(), 0, 0, BINDING_MODE::BIND_IN, op1.Item(8).buffer->b); // Item(3): Activations

	// === Forward path ===
	auto probs = dml::ActivationSigmoid(op2.Item(IT_INPUT));
	op2.AddIntermediate(probs); // Item(4)

	auto diff = dml::Subtract(probs, op2.Item(IT_LABELS));   // probs - labels
	auto mse = dml::Reduce(dml::Multiply(diff, diff), DML_REDUCE_FUNCTION_AVERAGE, { 1, 2, 3 });

	// === Resample back to activation size ===
	auto gradOut = dml::Resample(
		diff, { B, 4, wi / 4, he / 4 },
		DML_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
		DML_AXIS_DIRECTION_INCREASING
	);
	op2.AddIntermediate(gradOut); // Item(5)

	// === MatMul Prep ===
	const uint32_t N = 4;     // output classes
	const uint32_t K = 64;    // input channels
	const uint32_t M = (wi / 4) * (he / 4) * B; // spatial x batch

	// Aᵀ = activations (reshaped and transposed): [K, M]
	auto act2D = dml::Reinterpret(op2.Item(IT_INFROM), { M, K }, {});
	auto act2D_T = dml::Reinterpret(act2D, { K, M }, {});

	// B = gradient from logits: [M, N]
	auto grad2D = dml::Reinterpret(op2.Item(IT_GRADOUT), { M, N }, {});

	// Result: [K, N] = [64, 4]
	auto dLdW = dml2::MatMul(act2D_T, grad2D, false, false);
	auto reshaped = dml::Reinterpret(dLdW, { N, K, 1, 1 }, {});

	// SGD Update
	auto lr = ml.ConstantValueTensor(*op2.GetGraph(), 0.01f, { N, K, 1, 1 });
	auto scaled = dml::Multiply(reshaped, lr);
	auto updatedWeights = dml::Subtract(op2.Item(IT_INWEGHTS), scaled);

	// === Outputs ===
	op2.AddOutput(reshaped);       // dL/dW
	op2.AddOutput(mse);            // MSE
	op2.AddOutput(updatedWeights); // Updated weights (to share or save)

	op2.Build();
	return op2;
}


MLOP GetFPDMLOld(ML& ml,unsigned int B,unsigned int wi,unsigned int he)
{
	MLOP op1(&ml);

	// Inputs (ordered first!)
	op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { B, 1, wi, he } });      // Item(0) - Input image
	op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 8, 1, 3, 3 } });        // Item(1) - Conv1 weights
	op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 64, 8, 3, 3 } });       // Item(2) - Conv2 weights
	op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 4, 64, 1, 1 } });       // Item(3) - Final conv weights

	// Conv1
	auto conv1 = dml::Convolution(
		op1.Item(0), op1.Item(1),
		{}, DML_CONVOLUTION_MODE_CROSS_CORRELATION, DML_CONVOLUTION_DIRECTION_FORWARD,
		{ 1, 1 }, { 1, 1 }, { 1, 1 }, { 1, 1 }
	);
	op1.AddIntermediate(conv1);        // Item(4)

	auto relu1 = dml::ActivationRelu(op1.Item(4));
	op1.AddOutput(relu1);        // Item(5)

	auto pooled1 = dml::MaxPooling(op1.Item(5), { 2, 2 }, { 2, 2 }, { 0, 0 }, { 0, 0 });
	op1.AddIntermediate(pooled1.values); // Item(6)

	// Conv2
	auto conv2 = dml::Convolution(
		op1.Item(6), op1.Item(2),
		{}, DML_CONVOLUTION_MODE_CROSS_CORRELATION, DML_CONVOLUTION_DIRECTION_FORWARD,
		{ 1, 1 }, { 1, 1 }, { 1, 1 }, { 1, 1 }
	);
	op1.AddIntermediate(conv2);        // Item(7)

	auto relu2 = dml::ActivationRelu(op1.Item(7));
	op1.AddOutput(relu2);        // Item(8)

	auto pooled2 = dml::MaxPooling(op1.Item(8), { 2, 2 }, { 2, 2 }, { 0, 0 }, { 0, 0 });
	op1.AddOutput(pooled2.values); // Item(9)

	// Final conv
	auto convFinal = dml::Convolution(
		op1.Item(9), op1.Item(3),
		{}, DML_CONVOLUTION_MODE_CROSS_CORRELATION, DML_CONVOLUTION_DIRECTION_FORWARD,
		{ 1, 1 }, { 1, 1 }
	);
	op1.AddIntermediate(convFinal);    // Item(10)

	// Global Average Pool
	auto avg = dml::AveragePooling(
		op1.Item(10),
		{ 1, 1 },
		{ wi / 4, he / 4 },
		{ 0, 0 }, { 0, 0 },
		{ 1, 1 },
		false
	);

	// Final outputs (last!)
	op1.AddOutput(op1.Item(5));     // ReLU1 output (for backprop)
	op1.AddOutput(op1.Item(8));     // ReLU2 output
	op1.AddOutput(op1.Item(9));     // Pooled2 output (into final conv)
	op1.AddOutput(avg);             // Final logits → [B, 4, 1, 1]

	op1.Build();
	return op1;
}


MLOP GetFPDML(ML& ml, unsigned int B, unsigned int wi, unsigned int he)
{
	MLOP op1(&ml);

	// Inputs (ordered first!)
	op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { B, 1, wi, he } });       // Item(0) - Input image
	op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 8, 1, 3, 3 } });         // Item(1) - Conv1 weights
	op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 1,8,1,1 } });                  // Item(2) - Conv1 bias
	op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 64, 8, 3, 3 } });        // Item(3) - Conv2 weights
	op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 1,64,1,1 } });                 // Item(4) - Conv2 bias
	op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 4, 64, 1, 1 } });        // Item(5) - Final conv weights
	op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 1,4,1,1 } });                  // Item(6) - Final conv bias

	// Conv1
	auto conv1 = dml::Convolution(
		op1.Item(0), op1.Item(1), op1.Item(2),
		DML_CONVOLUTION_MODE_CROSS_CORRELATION, DML_CONVOLUTION_DIRECTION_FORWARD,
		{ 1, 1 }, { 1, 1 }, { 1, 1 }, { 1, 1 }
	);
	op1.AddIntermediate(conv1); // Item(7)

	auto relu1 = dml::ActivationRelu(op1.Item(7));
	op1.AddOutput(relu1); // Item(8)

	auto pooled1 = dml::MaxPooling(op1.Item(8), { 2, 2 }, { 2, 2 }, { 0, 0 }, { 0, 0 });
	op1.AddIntermediate(pooled1.values); // Item(9)

	// Conv2
	auto conv2 = dml::Convolution(
		op1.Item(9), op1.Item(3), op1.Item(4),
		DML_CONVOLUTION_MODE_CROSS_CORRELATION, DML_CONVOLUTION_DIRECTION_FORWARD,
		{ 1, 1 }, { 1, 1 }, { 1, 1 }, { 1, 1 }
	);
	op1.AddIntermediate(conv2); // Item(10)

	auto relu2 = dml::ActivationRelu(op1.Item(10));
	op1.AddOutput(relu2); // Item(11)

	auto pooled2 = dml::MaxPooling(op1.Item(11), { 2, 2 }, { 2, 2 }, { 0, 0 }, { 0, 0 });
	op1.AddOutput(pooled2.values); // Item(12)

	// Final Conv
	auto convFinal = dml::Convolution(
		op1.Item(12), op1.Item(5), op1.Item(6),
		DML_CONVOLUTION_MODE_CROSS_CORRELATION, DML_CONVOLUTION_DIRECTION_FORWARD,
		{ 1, 1 }, { 1, 1 }
	);
	op1.AddIntermediate(convFinal); // Item(13)

	// Global Average Pool
	auto avg = dml::AveragePooling(
		op1.Item(13),
		{ 1, 1 },
		{ wi / 4, he / 4 },
		{ 0, 0 }, { 0, 0 },
		{ 1, 1 },
		false
	);

	// Final outputs (last!)
	op1.AddOutput(op1.Item(8));   // ReLU1 output
	op1.AddOutput(op1.Item(11));  // ReLU2 output
	op1.AddOutput(op1.Item(12));  // Pooled2 output
	op1.AddOutput(avg);           // Final logits → [B, 4, 1, 1]

	op1.Build();
	return op1;

}


std::vector<TRAIN_CLASS> Inference(std::vector<std::wstring> files, unsigned int B = 1, unsigned int wi = 64, unsigned int he = 64)
{
	ML ml;
#ifdef _DEBUG
	ml.SetDebug(1);
#endif
	ml.On();
	auto op1 = GetFPDML(ml, B, wi, he);
	ml.ops.push_back(op1);

	std::vector<float> w1 = LoadBin((datafolder + std::wstring(L"\\net_0_weight.bin")).c_str());
	std::vector<float> w2 = LoadBin((datafolder + std::wstring(L"\\net_3_weight.bin")).c_str());
	std::vector<float> w3 = LoadBin((datafolder + std::wstring(L"\\net_6_weight.bin")).c_str());

	std::vector<float> b1 = LoadBin((datafolder + std::wstring(L"\\net_0_bias.bin")).c_str());
	std::vector<float> b2 = LoadBin((datafolder + std::wstring(L"\\net_3_bias.bin")).c_str());
	std::vector<float> b3 = LoadBin((datafolder + std::wstring(L"\\net_6_bias.bin")).c_str());

	ml.Prepare();

	// First conv
	op1.Item(1).buffer->Upload(&ml, w1.data(), w1.size() * sizeof(float));
	op1.Item(2).buffer->Upload(&ml, b1.data(), b1.size() * sizeof(float));

	// Second conv
	op1.Item(3).buffer->Upload(&ml, w2.data(), w2.size() * sizeof(float));
	op1.Item(4).buffer->Upload(&ml, b2.data(), b2.size() * sizeof(float));

	// Third conv
	op1.Item(5).buffer->Upload(&ml, w3.data(), w3.size() * sizeof(float));
	op1.Item(6).buffer->Upload(&ml, b3.data(), b3.size() * sizeof(float));

	std::vector<TRAIN_CLASS> results;
	for (auto& imgfile : files)
	{
		// Load image
		std::vector<float> image(wi * he);
		if (!LoadGrayscaleImageWIC(imgfile, wi, he, image.data()))
			return {};

		// Image
		op1.Item(0).buffer->Upload(&ml, image.data(), image.size() * sizeof(float));

		ml.Run();

		// Get the output
		std::vector<char> logits;
		op1.Item(17).buffer->Download(&ml, (size_t)-1, logits);

		// Convert logits to float
		std::vector<float> logitsFloat(logits.size() / sizeof(float));
		memcpy(logitsFloat.data(), logits.data(), logits.size());

		// Apply softmax to logitsFloat
		std::vector<float> probs(logitsFloat.size());
		float maxLogit = *std::max_element(logitsFloat.begin(), logitsFloat.end());

		// For numerical stability
		float sum = 0.0f;
		for (size_t i = 0; i < logitsFloat.size(); ++i) {
			probs[i] = std::exp(logitsFloat[i] - maxLogit);
			sum += probs[i];
		}
		for (float& p : probs)
			p /= sum;
		[[maybe_unused]] auto predicted = std::distance(probs.begin(), std::max_element(probs.begin(), probs.end()));
		results.push_back((TRAIN_CLASS)predicted);
	}

	return results;
}


/*
// DirectML based training doesn't work because DirectML lacks support for back propagation in convolution
void Train(unsigned int B = 1,unsigned int wi = 64, unsigned int he = 64)
{
	ML ml;
#ifdef _DEBUG
	ml.SetDebug(1);
#endif
	ml.On();

	// Forward operator
	MLOP op1(&ml);
	if (0)
	{
		op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { B,1,wi,he} }); // Input tensor
		op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 8,1,3,3} }); // Weights

		op1.AddIntermediate(
			dml::Convolution(
				op1.Item(0),             // input: [1, 1, 32, 32]
				op1.Item(1),             // weights: [8, 1, 3, 3]
				{}, DML_CONVOLUTION_MODE_CROSS_CORRELATION,
				DML_CONVOLUTION_DIRECTION_FORWARD,
				{ 1, 1 },
				{ 1,1 },
				{ 1,1 }, { 1,1 }
			));
		op1.AddOutput(dml::ActivationRelu(op1.Item(2))); // ReLU
		auto pooled = dml::MaxPooling(
			op1.Item(3),      // input: post-ReLU
			{ 2, 2 },           // kernel size
			{ 2, 2 },           // stride
			{ 0, 0 }, { 0, 0 }    // padding
		);
		op1.AddIntermediate(pooled.values);


		// Second convolution layer
		// Add weights for second conv layer
		op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 64, 8, 3, 3 } }); // Item(5)

		// Convolution (from pooled output: Item(4))
		op1.AddIntermediate(
			dml::Convolution(
				op1.Item(4),       // input: [1, 8, 16, 16]
				op1.Item(5),      // weights: [64, 8, 3, 3]
				{}, DML_CONVOLUTION_MODE_CROSS_CORRELATION,
				DML_CONVOLUTION_DIRECTION_FORWARD,
				{ 1, 1 },
				{ 1,1 },
				{ 1,1 }, { 1,1 }
			)
		); // Item(6)

		// ReLU
		op1.AddOutput(
			dml::ActivationRelu(op1.Item(6))
		); // Item(7)

		// MaxPool again
		auto pooled2 = dml::MaxPooling(
			op1.Item(7),       // input: [1, 64, 16, 16]
			{ 2, 2 }, { 2, 2 },
			{ 0, 0 }, { 0, 0 }
		);
		op1.AddOutput(pooled2.values); // Item(8) => [1, 64, 8, 8] 
		op1.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 4, 64, 1, 1 } }); // [out, in, 1, 1] = [4, 64, 1, 1]
		auto logitsConv = dml::Convolution(
			op1.Item(8),    // input: [1, 64, 8, 8]
			op1.Item(9),    // weights: [4, 64, 1, 1]
			{}, DML_CONVOLUTION_MODE_CROSS_CORRELATION,
			DML_CONVOLUTION_DIRECTION_FORWARD,
			{ 1, 1 },         // stride
			{ 1, 1 }         // dilation
		);
		op1.AddIntermediate(logitsConv); // Item(10) → [1, 4, 8, 8]
		auto avg = dml::AveragePooling(
			op1.Item(10),       // input: [1, 4, 8, 8]
			{ 1, 1 },             // strides
			{ wi/4, he/4 },             // windowSizes — covers entire spatial dimensions
			{ 0, 0 },             // startPadding
			{ 0, 0 },
			{ 1, 1 },             // dilations
			false// endPadding
		);
		op1.AddOutput(avg); // Item(11) → [1, 4, 1, 1]

		op1.Build();
		ml.ops.push_back(op1);
	}

	if (1)
	{

		
	}

	// And the Backprop Operator
	MLOP op2(&ml);
	if (0)
	{
		const int IT_INPUT = 0;
		const int IT_LABELS = 1;
		const int IT_INWEGHTS = 2;
		const int IT_INFROM = 3;
		const int IT_ACTS = 4;
		const int IT_GRADOUT = 5;

		op2.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { B, 4, 1, 1 } },0,8); // Logits → Item(0)
		op2.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, {B, 4, 1, 1 } }, 0, 8); // Labels → Item(1)
		// Add weights as an input (assuming {4, 64, 1, 1})
		op2.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { 4, 64, 1, 1 } }, 0, 8); // Item(6)
		// Add input from operator 1
		op2.AddInput(op1.Item(8).expr.GetOutputDesc(), 0, 0, BINDING_MODE::BIND_IN, op1.Item(8).buffer->b); // Item(5)

		auto probs = dml::ActivationSigmoid(op2.Item(IT_INPUT));
		op2.AddIntermediate(probs); // Item(2)

		auto diff = dml::Subtract(op2.Item(IT_ACTS), op2.Item(IT_LABELS));
		auto sqr = dml::Multiply(diff, diff);
		auto mse = dml::Reduce(sqr, DML_REDUCE_FUNCTION_AVERAGE, { 1, 2, 3 });

		auto shaped = dml::Reinterpret(diff, { B, 4, 1, 1 }, {});
		auto gradOut = dml::Resample(shaped, { B, 4, wi/4, he/4 }, DML_INTERPOLATION_MODE_NEAREST_NEIGHBOR, DML_AXIS_DIRECTION_INCREASING);
		op2.AddIntermediate(gradOut); // Item(4)


		auto grad2D = dml::Reinterpret(op2.Item(IT_GRADOUT), { B*(wi/4)*(he/4), 4}, {});
		auto act2D = dml::Reinterpret(op2.Item(IT_INFROM), { B*(wi/4)*(he/4), 64}, {});

		auto dLdW = dml2::MatMul(act2D, grad2D); // Result: [64, 4]
		auto reshaped = dml::Reinterpret(dLdW, { 4, 64, 1, 1 }, {});


		// Constant scalar learning rate
		auto lr = ml.ConstantValueTensor(*op2.GetGraph(), 0.01f, {4,64,1,1});

		// Scale gradient
		auto scaled = dml::Multiply(reshaped, lr);

		// Subtract from weights
		auto updatedWeights = dml::Subtract(op2.Item(IT_INWEGHTS), scaled);

		// Output stuff
		op2.AddOutput(reshaped); // ← This is your dL/dW!
		op2.AddOutput(mse); // Item(6)
		op2.AddOutput(updatedWeights); // Item(7)

		op2.Build();
	}


	if (1)
	{
		const int IT_LOGITS = 0;
		const int IT_LABELS = 1;
		const int IT_W1 = 2; // Conv1 weights [8,1,3,3]
		const int IT_W2 = 3; // Conv2 weights [64,8,3,3]
		const int IT_W3 = 4; // Final conv weights [4,64,1,1]
		const int IT_ACT1 = 5; // ReLU1 → [B, 8, wi, he]
		const int IT_ACT2 = 6; // ReLU2 → [B, 64, wi/2, he/2]
		const int IT_ACT3 = 7; // Pooled2 → [B, 64, wi/4, he/4]

		// Inputs
		op2.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { B, 4, 1, 1 } }, 0, 8); // logits
		op2.AddInput({ DML_TENSOR_DATA_TYPE_FLOAT32, { B, 4, 1, 1 } }, 0, 8); // labels

		op2.AddInput(op1.Item(1).expr.GetOutputDesc(), 0, false, BINDING_MODE::BIND_IN, op1.Item(1).buffer->b); // Conv1 weights
		op2.AddInput(op1.Item(2).expr.GetOutputDesc(), 0, false, BINDING_MODE::BIND_IN, op1.Item(2).buffer->b); // Conv2 weights
		op2.AddInput(op1.Item(3).expr.GetOutputDesc(), 0, false, BINDING_MODE::BIND_IN, op1.Item(3).buffer->b); // Final conv weights

		op2.AddInput(op1.Item(5).expr.GetOutputDesc(), 0, false, BINDING_MODE::BIND_IN, op1.Item(5).buffer->b); // ReLU1
		op2.AddInput(op1.Item(8).expr.GetOutputDesc(), 0, false, BINDING_MODE::BIND_IN, op1.Item(8).buffer->b); // ReLU2
		op2.AddInput(op1.Item(9).expr.GetOutputDesc(), 0, false, BINDING_MODE::BIND_IN, op1.Item(9).buffer->b); // Pooled2

		// ===== Loss computation =====
		auto probs = dml::ActivationSigmoid(op2.Item(IT_LOGITS));
		auto diff = dml::Subtract(probs, op2.Item(IT_LABELS));
		auto sqr = dml::Multiply(diff, diff);
		auto loss = dml::Reduce(sqr, DML_REDUCE_FUNCTION_AVERAGE, { 1, 2, 3 });

		// ===== Gradient reshaping =====
		auto gradReshaped = dml::Resample(diff, { B, 4, wi / 4, he / 4 }, DML_INTERPOLATION_MODE_NEAREST_NEIGHBOR, DML_AXIS_DIRECTION_INCREASING);

		// === dLdW3 ===
		auto grad2D_W3 = dml::Reinterpret(gradReshaped, { B * (wi / 4) * (he / 4), 4 }, {});
		auto act2D_W3 = dml::Reinterpret(op2.Item(IT_ACT3), { B * (wi / 4) * (he / 4), 64 }, {});
		auto act2D_T_W3 = dml::Reinterpret(act2D_W3, { 64, B * (wi / 4) * (he / 4) }, {});
		auto dLdW3 = dml2::MatMul(act2D_T_W3, grad2D_W3);              // [64, 4]
		auto dLdW3_r = dml::Reinterpret(dLdW3, { 4, 64, 1, 1 }, {});
		auto lr3 = ml.ConstantValueTensor(*op2.GetGraph(), 0.01f, { 4, 64, 1, 1 });
		auto updW3 = dml::Subtract(op2.Item(IT_W3), dml::Multiply(dLdW3_r, lr3));

		// === dLdW2 ===
		//		Step 1: Upsample the gradient from Con
 		auto gradUpsampled_W2 = dml::Resample(
			gradReshaped,                            // [B, 4, wi/4, he/4]
			{ B, 4, wi / 2, he / 2 },                // Upsample to Conv2 output spatial size
			DML_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
			DML_AXIS_DIRECTION_INCREASING
		);
		// Step 2: Flatten inputs for MatMul
		auto grad2D_W2 = dml::Reinterpret(gradUpsampled_W2, { B * (wi / 2) * (he / 2), 4 }, {});
		auto act2D_W2 = dml::Reinterpret(op2.Item(IT_ACT2), { B * (wi / 2) * (he / 2), 64 }, {});
		auto act2D_T_W2 = dml::Reinterpret(act2D_W2, { 64, B * (wi / 2) * (he / 2) }, {});
		// Step 3: MatMul → [64, 4]
		auto dLdW2_flat = dml2::MatMul(act2D_T_W2, grad2D_W2);
		// Step 4: Reshape to actual weight shape [64, 8, 3, 3]
		auto dLdW2 = dml::Reinterpret(dLdW2_flat, { 64, 8, 3, 3 }, {});
		// Step 5: Update weights
		auto lr2 = ml.ConstantValueTensor(*op2.GetGraph(), 0.01f, { 64, 8, 3, 3 });
		auto updW2 = dml::Subtract(op2.Item(IT_W2), dml::Multiply(dLdW2, lr2));



		// === dLdW1 ===
		auto grad2D_W1 = dml::Reinterpret(gradReshaped, { B * wi * he, 4 }, {});
		auto act2D_W1 = dml::Reinterpret(op2.Item(IT_ACT1), { B * wi * he, 8 }, {});
		auto act2D_T_W1 = dml::Reinterpret(act2D_W1, { 8, B * wi * he }, {});
		auto dLdW1 = dml2::MatMul(act2D_T_W1, grad2D_W1);              // [8, 4]
		auto dLdW1_r = dml::Reinterpret(dLdW1, { 8, 1, 3, 3 }, {});
		auto lr1 = ml.ConstantValueTensor(*op2.GetGraph(), 0.01f, { 8, 1, 3, 3 });
		auto updW1 = dml::Subtract(op2.Item(IT_W1), dml::Multiply(dLdW1_r, lr1));

		// ===== Outputs =====
		op2.AddOutput(updW1);   // Conv1 updated weights
		op2.AddOutput(updW2);   // Conv2 updated weights
		op2.AddOutput(updW3);   // Final conv updated weights
		op2.AddOutput(loss);    // MSE loss

		op2.Build();

	}

	ml.ops.push_back(op1);
	ml.ops.push_back(op2);
	ml.Prepare();


	// Input data
	std::vector<float> inputData(B * wi * he); // raw input
	std::vector<float> labelData(B * 4, 0.0f); // one-hot: tick, cross, dot, blank

	
	// Load the training set
	std::vector<std::wstring> files0 = LoadPngFilesFromDirectory(L"f:\\wuitools\\examAI\\python\\synthetic_marks_drawn\\0_empty\\");   // your list of image paths
	std::vector<std::wstring> files1 = LoadPngFilesFromDirectory(L"f:\\wuitools\\examAI\\python\\synthetic_marks_drawn\\1_tick\\");   // your list of image paths
	std::vector<std::wstring> files2 = LoadPngFilesFromDirectory(L"f:\\wuitools\\examAI\\python\\synthetic_marks_drawn\\2_cross\\");   // your list of image paths
	std::vector<std::wstring> files3 = LoadPngFilesFromDirectory(L"f:\\wuitools\\examAI\\python\\synthetic_marks_drawn\\3_dot\\");   // your list of image paths

	std::vector<int> labels;          // matching class labels (0–3)
	for (size_t i = 0; i < files0.size(); i++)
		labels.push_back(0);
	for (size_t i = 0; i < files1.size(); i++)
		labels.push_back(1);
	for (size_t i = 0; i < files2.size(); i++)
		labels.push_back(2);
	for (size_t i = 0; i < files3.size(); i++)
		labels.push_back(3);

	std::vector<std::wstring> files;
	files.insert(files.end(), files0.begin(), files0.end());
	files.insert(files.end(), files1.begin(), files1.end());
	files.insert(files.end(), files2.begin(), files2.end());
	files.insert(files.end(), files3.begin(), files3.end());

	std::vector<size_t> indices(files.size());
	std::iota(indices.begin(), indices.end(), 0);
	std::shuffle(indices.begin(), indices.end(), std::mt19937{ std::random_device{}() });


	for (unsigned int loop = 0; loop < files.size(); loop += B)
	{
		std::fill(labelData.begin(), labelData.end(), 0.0f);
		for (unsigned int b = 0; b < B && (loop + b) < indices.size(); ++b) 
		{
			// Load image for sample b
			float* dst = &inputData[b * wi * he];
			size_t idx = indices[loop + b];
			if (!LoadGrayscaleImageWIC(files[idx], wi, he, dst))
				DebugBreak();
			labelData[b * 4 + labels[idx]] = 1.0f;
		}
		op1.Item(0).buffer->Upload(&ml, inputData.data(), B * wi * he* sizeof(float));
		op2.Item(1).buffer->Upload(&ml, labelData.data(), labelData.size() * sizeof(float));
		ml.Run();
	}

}

*/

std::mt19937 rng(std::random_device{}());

/*
 Not yet working

void Train(unsigned int B = 1, unsigned int wi = 64, unsigned int he = 64,int EPOCHS = 40)
{

	ML ml;
#ifdef _DEBUG
	ml.SetDebug(1);
#endif
	ml.On();

	// Forward operator
	MLOP op1 = GetFPDML(ml, B, wi, he);
	MLOP op2 = GetBPDML(ml, op1, B, wi, he);

	ml.ops.push_back(op1);
	ml.ops.push_back(op2);
	ml.Prepare();


	// Input data
	std::vector<float> inputData(B * wi * he); // raw input
	std::vector<float> labelData(B * 4, 0.0f); // one-hot: tick, cross, dot, blank


	// Load the training set
	std::vector<std::wstring> files0 = LoadPngFilesFromDirectory(L"f:\\wuitools\\examAI\\python\\synthetic_marks_drawn\\0_empty\\");   // your list of image paths
	std::vector<std::wstring> files1 = LoadPngFilesFromDirectory(L"f:\\wuitools\\examAI\\python\\synthetic_marks_drawn\\1_tick\\");   // your list of image paths
	std::vector<std::wstring> files2 = LoadPngFilesFromDirectory(L"f:\\wuitools\\examAI\\python\\synthetic_marks_drawn\\2_cross\\");   // your list of image paths
	std::vector<std::wstring> files3 = LoadPngFilesFromDirectory(L"f:\\wuitools\\examAI\\python\\synthetic_marks_drawn\\3_dot\\");   // your list of image paths

	std::vector<int> labels;          // matching class labels (0–3)
	for (size_t i = 0; i < files0.size(); i++)
		labels.push_back(0);
	for (size_t i = 0; i < files1.size(); i++)
		labels.push_back(1);
	for (size_t i = 0; i < files2.size(); i++)
		labels.push_back(2);
	for (size_t i = 0; i < files3.size(); i++)
		labels.push_back(3);

	std::vector<std::wstring> files;
	files.insert(files.end(), files0.begin(), files0.end());
	files.insert(files.end(), files1.begin(), files1.end());
	files.insert(files.end(), files2.begin(), files2.end());
	files.insert(files.end(), files3.begin(), files3.end());

	std::vector<size_t> indices(files.size());
	std::iota(indices.begin(), indices.end(), 0);


	for (int epoch = 0; epoch < EPOCHS; ++epoch)
	{
		std::shuffle(indices.begin(), indices.end(), std::mt19937{ std::random_device{}() });
		for (unsigned int loop = 0; loop < files.size(); loop += B)
		{
			std::fill(labelData.begin(), labelData.end(), 0.0f);
			for (unsigned int b = 0; b < B && (loop + b) < indices.size(); ++b)
			{
				// Load image for sample b
				float* dst = &inputData[b * wi * he];
				size_t idx = indices[loop + b];
				if (!LoadGrayscaleImageWIC(files[idx], wi, he, dst))
					DebugBreak();
				labelData[b * 4 + labels[idx]] = 1.0f;
			}
			op1.Item(0).buffer->Upload(&ml, inputData.data(), B * wi * he * sizeof(float));
			op2.Item(1).buffer->Upload(&ml, labelData.data(), labelData.size() * sizeof(float));

			if (epoch == 0 && loop == 0)
			{
				// Conv1: [8, 1, 3, 3]
				
				std::uniform_real_distribution<float> dist(-0.1f, 0.1f);
				std::vector<float> w1_init(8 * 1 * 3 * 3);
				for (auto& w : w1_init) w = dist(rng);
				op1.Item(1).buffer->Upload(&ml, w1_init.data(), w1_init.size() * sizeof(float));

				// Conv2: [64, 8, 3, 3]
				std::vector<float> w2_init(64 * 8 * 3 * 3);
				for (auto& w : w2_init) w = dist(rng);
				op1.Item(2).buffer->Upload(&ml, w2_init.data(), w2_init.size() * sizeof(float));
			}

			if (epoch == 0 && loop == 0)
			{


				std::vector<float> w_init(4 * 64);
				std::uniform_real_distribution<float> dist(-0.1f, 0.1f);
				for (auto& w : w_init) w = dist(rng);

				op1.Item(3).buffer->Upload(&ml, w_init.data(), w_init.size() * sizeof(float));
			}

			ml.Run();

			if (1)
			{
				std::vector<char> dLdWData;
				op2.Item(6).buffer->Download(&ml, (size_t)-1, dLdWData);

				const float* dLdWf = reinterpret_cast<const float*>(dLdWData.data());
				float sum = 0;
				for (size_t i = 0; i < dLdWData.size() / sizeof(float); ++i)
					sum += std::abs(dLdWf[i]);
			}

			if (1)
			{
				std::vector<char> updatedWeights;
				op2.Item(8).buffer->Download(&ml, (size_t)-1, updatedWeights);
				op1.Item(3).buffer->Upload(&ml, updatedWeights.data(), updatedWeights.size());
			}

			std::vector<char> loss;
			op2.Item(7).buffer->Download(&ml, (size_t)-1,loss);
			float* floss = (float*)loss.data();
			if (floss > 0)
				MessageBeep(0);

		}
	}


	std::vector<char> finalWeights(4 * 64 * 1 * 1 * 4); // 1024 floats
	op2.Item(7).buffer->Download(&ml,(size_t)-1, finalWeights);
	SaveBin(L"net6_weight.bin", finalWeights);

}

*/