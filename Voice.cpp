#include "Voice.h"
#include "ext/ZCharScanner.h"

std::vector<int32_t> Voice::CharsToID(const std::string & RawInTxt)
{

    std::vector<int32_t> VecPhones;

    std::u32string InTxt = VoxUtil::StrToU32(RawInTxt);

    for (const auto& Char : InTxt)
    {
        size_t ArrID = 0;
        std::u32string CharAs;
        CharAs += Char;

        if (VoxUtil::FindInVec<std::u32string>(CharAs, Phonemes, ArrID))
            VecPhones.push_back(PhonemeIDs[ArrID]);
        else
            std::cout << "Voice::PhonemesToID() WARNING: Unknown phoneme " << Char << std::endl;



    }


    return VecPhones;

}

std::vector<int32_t> Voice::PhonemesToID(const std::string & RawInTxt)
{
    ZStringDelimiter Delim(RawInTxt);
	Delim.AddDelimiter(" ");
    std::u32string InTxt = VoxUtil::StrToU32(RawInTxt);


	std::vector<int32_t> VecPhones;
	VecPhones.reserve(Delim.szTokens());


	for (const auto& Pho : Delim.GetTokens()) 
	{
		size_t ArrID = 0;
        std::u32string PhnU = VoxUtil::StrToU32(Pho);

        if (VoxUtil::FindInVec<std::u32string>(PhnU, Phonemes, ArrID))
            VecPhones.push_back(PhonemeIDs[ArrID]);
		else
            std::cout << "Voice::PhonemesToID() WARNING: Unknown phoneme " << Pho << std::endl;



	}


	return VecPhones;

}

void Voice::ReadPhonemes(const std::string &PhonemePath)
{
    std::ifstream Phone(PhonemePath);

    std::string Line;
    while (std::getline(Phone, Line))
    {
        if (Line.find("\t") == std::string::npos)
            continue;


        ZStringDelimiter Deline(Line);
        Deline.AddDelimiter("\t");

        Phonemes.push_back(VoxUtil::StrToU32(Deline[0]));
        PhonemeIDs.push_back(stoi(Deline[1]));




    }

}

void Voice::ReadSpeakers(const std::string &SpeakerPath)
{
    Speakers = VoxUtil::GetLinedFile(SpeakerPath);

}

void Voice::ReadEmotions(const std::string &EmotionPath)
{
    Emotions = VoxUtil::GetLinedFile(EmotionPath);

}


void Voice::ReadModelInfo(const std::string &ModelInfoPath)
{

    ModelInfo = "";
    std::vector<std::string> MiLines = VoxUtil::GetLinedFile(ModelInfoPath);

    for (const std::string& ss : MiLines)
        ModelInfo += ss + "\n";


}


Voice::Voice(const std::string & VoxPath, const std::string &inName, Phonemizer *InPhn)
{
    ReadModelInfo(VoxPath + "/info.txt");

    VoxInfo = VoxUtil::ReadModelJSON(VoxPath + "/info.json");

    if (VoxInfo.Architecture.Text2Mel == EText2MelModel::Tacotron2)
        MelPredictor = std::make_unique<Tacotron2>();
    else
        MelPredictor = std::make_unique<FastSpeech2>();


    MelPredictor->Initialize(VoxPath + "/melgen",(ETTSRepo::Enum)VoxInfo.Architecture.Repo);


    Vocoder.Initialize(VoxPath + "/vocoder");

    if (InPhn)
        Processor.Initialize(InPhn);


    Name = inName;
    ReadPhonemes(VoxPath + "/phonemes.txt");
    ReadSpeakers(VoxPath + "/speakers.txt");
    ReadEmotions(VoxPath + "/emotions.txt");









}

void Voice::AddPhonemizer(Phonemizer *InPhn)
{
    Processor.Initialize(InPhn);


}

std::string Voice::PhonemizeStr(const std::string &Prompt)
{


    return Processor.ProcessTextPhonetic(Prompt,Phonemes,CurrentDict,
                                                            (ETTSLanguage::Enum)VoxInfo.Language,
                                                           true); // default voxistac to true to preserve punctuation.

}


VoxResults Voice::Vocalize(const std::string & Prompt, float Speed, int32_t SpeakerID, float Energy, float F0, int32_t EmotionID)
{



    bool VoxIsTac = VoxInfo.Architecture.Text2Mel == EText2MelModel::Tacotron2;

    std::string PromptToFeed = Prompt;
    if (VoxInfo.Language > -1)
        PromptToFeed += VoxInfo.EndPadding;

    std::string PhoneticTxt = Processor.ProcessTextPhonetic(PromptToFeed,Phonemes,CurrentDict,
                                                            (ETTSLanguage::Enum)VoxInfo.Language,
                                                           VoxIsTac);
    TFTensor<float> Mel;
    TFTensor<float> Attention;

    std::vector<int32_t> InputIDs;


    // Note to self: always check for negative or positive language by checking that it is lower than 0
    // if we try greater than 0, English is missed.
    if (VoxInfo.Language < 0){
        InputIDs = CharsToID(PhoneticTxt);
        InputIDs.push_back(std::stoi(VoxInfo.EndPadding));


    }
    else
    {
        if (VoxInfo.s_Language.find("IPA") != std::string::npos)
            InputIDs = CharsToID(PhoneticTxt);
        else
            InputIDs = PhonemesToID(PhoneticTxt);




    }

    std::vector<float> FloatArgs;
    std::vector<int32_t> IntArgs;


    if (VoxIsTac)
    {

        Mel = ((Tacotron2*)MelPredictor.get())->DoInference(InputIDs,FloatArgs,IntArgs,SpeakerID, EmotionID);
        Attention = ((Tacotron2*)MelPredictor.get())->Attention;

    }
    else
    {

        FloatArgs = {Speed,Energy,F0};

        Mel = ((FastSpeech2*)MelPredictor.get())->DoInference(InputIDs,FloatArgs,IntArgs,SpeakerID, EmotionID);

    }


	TFTensor<float> AuData = Vocoder.DoInference(Mel);
    std::vector<float> AudioData;

    if (AuData.Shape.size() > 1)
    {

        int64_t Width = AuData.Shape[0];
        int64_t Height = AuData.Shape[1];
        int64_t Depth = AuData.Shape[2];
        //int z = 0;


        AudioData.resize(Height);

        // Code to access 1D array as if it were 3D
        for (int64_t x = 0; x < Width;x++)
        {
            for (int64_t z = 0;z < Depth;z++)
            {
                for (int64_t y = 0; y < Height;y++) {
                    int64_t Index = x * Height * Depth + y * Depth + z;
                    AudioData[(size_t)y] = AuData.Data[(size_t)Index];

                }

            }
        }

    }
    else
    {
        // Pre-flattened vocoder output
        AudioData = AuData.Data;

    }




    return {AudioData,Attention,Mel};
}

void Voice::SetDictEntries(const std::vector<DictEntry> &InEntries)
{
    for (const DictEntry& Entr : InEntries)
    {
        if (Entr.Language != VoxInfo.s_Language)
            continue;

        CurrentDict.push_back(Entr);

    }

}

Voice::~Voice()
{
}
