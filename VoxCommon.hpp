#pragma once
/*
 VoxCommon.hpp : Defines common data structures and constants to be used with TensorVox 
*/
#include <iostream>
#include <vector>
#include "ext/AudioFile.hpp"
#include "ext/CppFlow/ops.h"
#include "ext/CppFlow/model.h"
#include <QMessageBox>

#define IF_RETURN(cond,ret) if (cond){return ret;}

const uint32_t CommonSampleRate = 48000;

// https://github.com/almogh52/rnnoise-cmake/blob/d981adb2e797216f456cfcf158f73761a29981f8/examples/rnnoise_demo.c#L31
const uint32_t RNNoiseFrameSize = 480;
typedef std::vector<std::tuple<std::string,cppflow::tensor>> TensorVec;

template<typename T>
struct TFTensor {
	std::vector<T> Data;
	std::vector<int64_t> Shape;
	size_t TotalSize;

};


namespace ETTSRepo {
enum Enum{
    TensorflowTTS = 0,
    CoquiTTS
};

}
namespace EText2MelModel {
enum Enum{
    FastSpeech2 = 0,
    Tacotron2
};

}

namespace EVocoderModel{
enum Enum{
    MultiBandMelGAN = 0,
    MelGANSTFT // there is no architectural changes so we can use mb-melgan class for melgan-stft
};
}

// Negative numbers denote character-based language, positive for phoneme based. Standard is char-equivalent language idx = negative(phn-based)
// In case of English, since -0 doesn't exist, we use -1.
// For example, German phonetic would be 3, and character based would be -3
// IPA-phn-based are mainly for Coqui
namespace ETTSLanguage{
enum Enum{
  GermanChar = -3,
  SpanishChar,
  EnglishChar,
  EnglishPhn,
  SpanishPhn,
  GermanPhn,
  EnglishIPA,
};

}



struct ArchitectureInfo{
    int Repo;
    int Text2Mel;
    int Vocoder;

    // String versions of the info, for displaying.
    // We want boilerplate int index to str conversion code to be low.
    std::string s_Repo;
    std::string s_Text2Mel;
    std::string s_Vocoder;

};
struct VoiceInfo{
  std::string Name;
  std::string Author;
  int32_t Version;
  std::string Description;
  ArchitectureInfo Architecture;
  std::string Note;

  uint32_t SampleRate;

  int32_t Language;
  std::string s_Language;
  std::string s_Language_Num;

  std::string EndPadding;



};

namespace VoxUtil {

    std::string U32ToStr(const std::u32string& InU32);
    std::u32string StrToU32(const std::string& InStr);

    std::vector<std::string> GetLinedFile(const std::string& Path);

    VoiceInfo ReadModelJSON(const std::string& InfoFilename);


	template<typename F>
    TFTensor<F> CopyTensor(cppflow::tensor& InTens)
	{
		std::vector<F> Data = InTens.get_data<F>();
        std::vector<int64_t> Shape = InTens.shape().get_data<int64_t>();
		size_t TotalSize = 1;
		for (const int64_t& Dim : Shape)
			TotalSize *= Dim;

		return TFTensor<F>{Data, Shape, TotalSize};


	}

	template<typename V>
	bool FindInVec(V In, const std::vector<V>& Vec, size_t& OutIdx, size_t start = 0) {
		for (size_t xx = start;xx < Vec.size();xx++)
		{
			if (Vec[xx] == In) {
				OutIdx = xx;
				return true;

			}

		}


		return false;

	}
    template<typename V, typename X>
    bool FindInVec2(V In, const std::vector<X>& Vec, size_t& OutIdx, size_t start = 0) {
        for (size_t xx = start;xx < Vec.size();xx++)
        {
            if (Vec[xx] == In) {
                OutIdx = xx;
                return true;

            }

        }


        return false;

    }

	void ExportWAV(const std::string& Filename, const std::vector<float>& Data, unsigned SampleRate);
}
