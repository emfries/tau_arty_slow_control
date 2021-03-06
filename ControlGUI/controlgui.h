#ifndef CONTROLGUI_H
#define CONTROLGUI_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include "qcustomplot.h"
#include <QStringList>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <functional>

extern "C" {
#include "u3.h"
}

namespace Ui {
class ControlGUI;
}

template <class state_type> class ControlEvent
{
public:
    ControlEvent(double tval, state_type stval,double btval) {
        t = tval; st = stval; bt = btval;
    }
    double time() const { return t; }
    double bltime() const {return bt;}
    state_type state() const { return st; }

private:
    double t;
    state_type st;
    double bt;
};

class FpgaAction
{
public:
    FpgaAction(uint32_t ticks0, uint32_t ticks1, int fpga_channel, bool channel_state) {
        t0 = ticks0; t1 = ticks1; ch = fpga_channel; st = channel_state;
    }
    uint32_t ticks0() const { return t0; }
    uint32_t ticks1() const { return t1; }
    int fpga_channel() const { return ch; }
    bool channel_state() const { return st; }

private:
    uint32_t t0;
    uint32_t t1;
    int ch;
    bool st;
};

class FpgaState
{
public:
    FpgaState(uint32_t ticks0, uint32_t ticks1, uint16_t fpga_state) {
        t0 = ticks0; t1 = ticks1; st = fpga_state;
    }
    uint32_t ticks0() const { return t0; }
    uint32_t ticks1() const { return t1; }
    uint16_t fpga_state() const { return st; }

private:
    uint32_t t0;
    uint32_t t1;
    uint16_t st;    
};

class ControlGUI : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit ControlGUI(QWidget *parent = 0);
    ~ControlGUI();

    // These are the FPGA digital IO channels.
    // Set the values in the class implementation file.
    static const int kD2PumpOutCh;
    static const int kSourceCtrlCh;
    static const int ktauOKCh;
    static const int kBeamEnableCh;
    static const int kWestGateValveCh;
    static const int kCleanerCh;
    static const int kRoundhouseGateValveCh;
    static const int kDaqTriggerCh; //starts MIDAS DAQ
    static const int kButterflyCh; // butterfly valve at input to cross assembly
    static const int kTrapdoorCh1;
    static const int kTrapdoorCh2;
    static const int kDaggerCh;
    static const int kGiantCleanerCh;
    static const int kGoLineCh; // from MPA3, tells if DAQ is running
    static const int kHmGXCh; // H-GX from CCR, stretched to 1 s pulses via gate generator
    static const int kAbortCh;

    static const QString kControlDir; // directory to find the midas start and elog post scripts

    // Logic sense of the control channels.
    //   1 ("true") means FPGA output HIGH (+3.3V).
    // Set the values in the class implementation file.
    static const bool kPumpOutPortOpen;
    static const bool kSourceCtrlOn;
    static const bool ktauOK;
    static const bool kBeamOn;
    static const bool kWestGateValveOpen;
    static const bool kCleanerDown;
    static const bool kRoundhouseGateValveOpen;
    static const bool kDaqRunning;
    static const bool kButterflyClosed;
    static const bool kDaggerPulse;
    static const bool kGiantCleanerDownOrPulse;
    static const bool kGoLineGood;
    static const bool kHmGXSignal;
    static const bool kAbortSignal;
    
    // Trapdoor options: must be 0,1,2,3
    // More significant bit is sent to kTrapdoorCh1
    // Less significant bit is sent to kTrapdoorCh2
    //   1 ("true") means FPGA output HIGH (+3.3V)
    static const int kTrapdoorNone;
    static const int kTrapdoorFill;
    static const int kTrapdoorHold;
    static const int kTrapdoorDump;
    
    static const int kPulseChannels;
    static const int kPulseLow;
    static const int kPulseTimeMS;
    static const int kBlockTimeMS;
    
    // Lead/lags for enabling/disabling nEDM
    static const double ktauOKLag;
    static const double ktauOKLead;
    
private slots:
    void dagger_button_pushed();
    void giantCleaner_button_pushed();
    
    void something_toggled(QList<FpgaState> states); // helper for all below toggled methods
    void westGateValve_toggled(bool checked);
    void roundhouseGateValve_toggled(bool checked);
    void beamEnable_toggled(bool checked);
    void nedmEnable_toggled(bool checked);
    void cleaner_toggled(bool checked);
    void trapdoor_index_changed(int index);
    void butterfly_toggled(bool checked);
    void giantCleaner_toggled(bool checked);

    void openFile_clicked();
    void startButton_clicked(bool firstcall=true, bool updatePattern=true);
    void updateElapsedTime();
    void abortButton_clicked();
    
    // transition methods
    void waitForGoThenStart(bool firstcall=false);
    void doNextBeamTransition(bool firstcall=false);
    void doNextWestGateValveTransition(bool firstcall=false);
    void doNextRoundhouseGateValveTransition(bool firstcall=false);
    void doNextCleanerTransition(bool firstcall=false);
    void doNextTrapdoorTransition(bool firstcall=false);
    void doNextDaggerTransition(bool firstcall=false);
    void doNextGiantCleanerTransition(bool firstcall=false);
    void doNextButterflyTransition(bool firstcall=false);
    void doNextnEDMTransistion(bool firstcall=false);
    void doNextPumpOutPortTransistion(bool firstcall=false);
    void doNextSourceCtrlTransistion(bool firstcall=false);

    void on_loadfileNumber_button_clicked();
	
	void fillCheck();

    //void on_D2PumpOutradioButton_toggled(bool checked);

    void enableChannelButtons(bool state);

private:
    Ui::ControlGUI *ui;

    // Following the u3EFunctions.c example...
    HANDLE hDevice;
    u3CalibrationInfo caliInfo;
    int localID;

    bool updateTimingPattern(QString filename);
    void updatePlot();
    //void enableChannelButtons(bool state);
    void disableChannelButtons();
    QTimer* SetMyTimer(bool isSingleShot ,
                    bool isPrecise,const char* typeofConnection);

    template<typename statetype> void addToGraph(const char* gName,QList<ControlEvent<statetype> > states,
                    double offset,double scale, double xmax,const QColor color,
                    QCPGraph *gGraph);    
    
    void get_zdagger();
    void reset_dagger_velacceldecel();
    void get_zGiantCleaner();
    void reset_giantCleaner_velacceldecel();
    void get_trapdoor_state();
    void get_fpga_params();
    
    bool fpga_run_in_progress;
    uint16_t fpga_current_state, fpga_initial_state;
    uint64_t fpga_max_ticks; // would use uint32_t but the value is supposed to be 2**32
    uint32_t fpga_timer_freq;
    int fpga_output_length;
    void convert_time_to_ticks(double time, uint32_t &ticks0, uint32_t &ticks1);
    void set_nth_bit(uint16_t &data, uint16_t n, bool high_or_low);
    bool load_fpga(QList<FpgaState> states);
    
    void get_midas_odb();
    double factor;
    double deltat;
    double dagger_up_time;
    double count_time;
    double clean_time;
    int run_number;
    bool override_list;
    
    bool start_midas();
    bool start_robs_daq();
    bool stop_robs_daq();
    bool setdb(double ft,double ct);
    void make_elog_post();
    void startTheCycle();
    void stopTheCycle(bool abort_run = false, bool bad_fill=false);

    void InitTimers();
    void ConnectSlots();

    int iStart; // index to keep track of which parameter file to use
    int sync_count; // counts how many times in a row the FPGA has been out of sync
    double last_check_time; // when is the last time Slow Control can check if FPGA is synched

    // indicies to keep track of the next transitions
    int iBeamTransition;
    int iWestGateValveTransition;
    int iRoundhouseGateValveTransition;
    int iCleanerTransition;
    int iTrapdoorTransition;
    int iDaggerTransition;
    int iGiantCleanerTransition;
    int iButterflyTransition;
    int inEDMTransition;
    int iPumpoutPortTransistion;
    int iSourceCtrlTransistion;

    QList<ControlEvent<bool> >    beamEnableStates;
    QList<ControlEvent<bool> >    westGateValveStates;
    QList<ControlEvent<bool> >    roundhouseGateValveStates;
    QList<ControlEvent<bool> >    cleanerStates;
    QList<ControlEvent<QString> > trapdoorStates;
    QList<ControlEvent<double> >  daggerPositions;
    QList<ControlEvent<double> >  giantCleanerPositions;
    QList<ControlEvent<bool> >    butterflyStates;
    QList<ControlEvent<bool> >    nEDMstates;
    QList<ControlEvent<bool> >    PumpOutStates;
    QList<ControlEvent<bool> >    SourceCtrlStates;
    
    QList<QString> daggerQueue;
    QList<QString> giantCleanerQueue;
    
    QList<FpgaAction> fpgaActions;
    QList<FpgaState> fpgaStates;

    QList<QTimer*> TimerList;

    QTimer *waitForGoTimer;
    // Event timers
    QTimer *beamTransitionTimer;
    QTimer *westGateValveTransitionTimer;
    QTimer *roundhouseGateValveTransitionTimer;
    QTimer *cleanerTransitionTimer;
    QTimer *trapdoorTransitionTimer;
    QTimer *daggerTransitionTimer;
    QTimer *giantCleanerTransitionTimer;
    QTimer *butterflyTransitionTimer;
    QTimer *pumpoutportTimer;
    QTimer *nEDMGV_Timer;
    QTimer *sourceCtrlTimer;

    QTimer *elapsedTimer;
    QCPItemLine *progressLine;
	
	QTimer *fillCheckTimer;

    QTime cycleTime;

    void *context;
    void *socket_trapdoor;
    void *socket_dagger;
    void *socket_giantCleaner;
    void *socket_daq;
    void *socket_fpga;
	void *socket_fillCheck;
    static const int dagger_recv_buffer_size = 256;
    char dagger_recv_buffer[dagger_recv_buffer_size];
    static const int giantCleaner_recv_buffer_size = 256;
    char giantCleaner_recv_buffer[giantCleaner_recv_buffer_size];
    static const int trapdoor_recv_buffer_size = 256;
    char trapdoor_recv_buffer[trapdoor_recv_buffer_size];
    static const int daq_recv_buffer_size = 256;
    char daq_recv_buffer[daq_recv_buffer_size];
	static const int fpga_recv_buffer_size = 256;
    char fpga_recv_buffer[fpga_recv_buffer_size];
	static const int fillCheck_recv_buffer_size = 256;
    char fillCheck_recv_buffer[fillCheck_recv_buffer_size];

    QStringList runFileNames;
};

#endif // CONTROLGUI_H
