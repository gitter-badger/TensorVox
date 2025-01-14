#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Voice.h"
#include <QStringList>
#include <QAudioFormat>
#include "voicemanager.h"
#include <QBuffer>
#include <QListWidgetItem>
#include <QAudio>
#include <QAudioOutput>
#include <queue>

#include "phonetichighlighter.h"

#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>

#include "phoneticdict.h"
#include "batchdenoisedlg.h"
#include "rnnoise.h"
#include <QClipboard>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct InferDetails{
  QString Prompt;
  float Energy;
  float Speed;
  float F0;
  float Amplification;
  QListWidgetItem* pItem;
  int32_t SpeakerID;
  int32_t EmotionID;
  QString VoiceName;
  bool Denoise;

  uint32_t SampleRate;

  std::vector<float> ForcedAudio;
  QString ExportFileName;





};
struct InferIDTrueID{
  uint32_t first;
  size_t second;

  int32_t Align;

};






class MainWindow : public QMainWindow
{
    Q_OBJECT

private:

    std::vector<InferIDTrueID> IdVec;

    std::vector<TFTensor<float>> Alignments;
    std::vector<TFTensor<float>> Mels;

    VoiceManager VoMan;
    QAudioFormat StdFmt;
    QAudioOutput* StdOutput;

    QWinTaskbarButton* pTaskBtn;

    QWinTaskbarProgress* pTskProg;


    int32_t CurrentInferIndex;
    uint32_t CurrentAmtThreads;
    PhoneticDict PhonDict;

    QClipboard* ClipBrd;

    uint32_t LastInferBatchSize;

    bool DoUpdateSplitAuto;

    InferIDTrueID* FindByFirst(uint32_t inGetID);
    QString LastExportDir;



protected:
    friend class BatchDenoiseDlg;

    void PushToInfers(InferDetails& InDets);
    int32_t GetCountItems();
    bool DoingBatchDenoising;

    int32_t DenBatchSize;
    int32_t DenDone;

public:
    void* pDarkFw;
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();
protected:
   void showEvent(QShowEvent *e) override;
public slots:
    void OnAudioRecv(std::vector<float> InDat,TFTensor<float> InMel,std::chrono::duration<double> infer_span,uint32_t inID);
    void OnAudioStateChange(QAudio::State newState);
    void OnClipboardDataChanged();

    void OnAttentionRecv(TFTensor<float> InAtt,uint32_t inID);
private slots:

    void on_btnInfer_clicked();

    void on_btnLoad_clicked();

    void on_cbModels_currentIndexChanged(const QString &arg1);

    void on_sliEnergy_sliderMoved(int position);

    void on_sliSpeed_sliderMoved(int position);

    void on_lstUtts_itemDoubleClicked(QListWidgetItem *item);

    void on_btnClear_clicked();

    void on_btnExportSel_clicked();

    void on_actionExport_performance_report_triggered();

    void on_chkRecPerfLog_clicked(bool checked);

    void on_btnExReport_clicked();

    void on_btnRefreshList_clicked();

    void on_sliF0_sliderMoved(int position);

    void on_cbModels_currentTextChanged(const QString &arg1);

    void on_hkInfer_triggered();

    void on_actionOverrides_triggered();

    void on_actionRefresh_model_listing_triggered();

    void on_btnLoadInfo_clicked();

    void on_chkMultiThreaded_stateChanged(int arg1);

    void OnFireLogiLed();

    void on_actDenWAV_triggered();

    void on_actShowWaveform_triggered();

    void on_actShowWaveform_toggled(bool arg1);

    void on_tabMetrics_currentChanged(int index);

    void on_actExAtt_triggered();

    void on_actExSpec_triggered();

    void OnMemoryUpdate();

    void on_actionPhonemize_filelist_triggered();

    void on_actPhnSel_triggered();

    void on_spbSeqLen_editingFinished();

    void on_actBatchDen_triggered();

    void on_actStopPlaying_triggered();

private:

    bool AllowedToPlayAudio();


    Voice* GetCurrentVoice();


    void AutoUpdateSplit();
    QString PhonemizeStr(QString &Text, Voice& VoxIn);
    QLabel* StatusLbl;

    size_t GetMemoryUsage();
    InferIDTrueID *FindBySecond(int32_t BuffID);

    void UpdateIfDoSlides();
    void PlotSpec(const TFTensor<float>& InMel, float TimeInSecs);
    void PlotAttention(const TFTensor<float> &TacAtt);

    void ExportAudBuffer(const QString& InFilename,const QByteArray& CurrentBuff,uint32_t InSampleRate);

    bool LogiLedAvailable;

    void ResetLogiLedIn(unsigned secs);

    int32_t NumDone;
    int32_t GetID(int32_t InID);
    void UpdateLogiLed();
    void SetDict();
    void HandleIsMultiSpeaker(size_t inVid);
    void HandleIsMultiEmotion(size_t inVid);
    bool CanPlayAudio;
    QStringList ListDirs(const QString& ParentDir);
    float RangeToFloat(int val);
    void PlayBuffer(QBuffer* pBuff, bool ByUser, int32_t RowID);
    bool RecPerfLines;

    void AdvanceBuffer();
    void AdvanceQueue();

    int32_t CountBlues();

    int32_t GetNumThreads();

    bool MustExplicitlyIterateQueue();
    void PopulateComboBox();

    std::vector<QBuffer*> AudBuffs;

    size_t CurrentBuffIndex;

    QStringList SuperWordSplit(const QString& InStr,int MaxLen);

    void ProcessCurlies(QString& ModTxt);


    std::queue<InferDetails> Infers;

    QStringList PerfReportLines;
    void IterateQueue();

    void DoInference(InferDetails &Dets);

    PhoneticHighlighter* pHigh;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
