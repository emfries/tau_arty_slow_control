#include "controlgui.h"
#include "ui_controlgui.h"
#include <QMessageBox>
#include <zmq.h>
#include <QFileDialog>
#include <assert.h>
#include <QProcess>
#include <errno.h>
#include <QDateTime>
#include <QTimer>
#include <vector>

#define TESTFPGA 0
#define TESTDAGGER 1
#define TESTGIANTCLEANER 1
#define TESTTRAPDOOR 1
#define TESTDAQ 0
#define BLIND 1
#define DEBUG_BLIND 0
#define GCFULLACTUATORINUSE 0 // 0 if using a simple solenoid, 1 if using the actuator driven by a motor
#define FILLCHECK 0

// n.b.: FPGA can only handle ~32 data points, don't try to load more than that, TODO: improve this later

// done automate fpga_server.py to find the right /dev/ttyUSB*, http://hintshop.ludvig.co.nz/show/persistent-names-usb-serial-devices/

const QString ControlGUI::kControlDir = "/home/daq/SlowControl/tau_arty_slow_control-master/";

// These are the FPGA IO channels
// n.b.: The numbering here starts from 0. The numbering on the tagbit wires starts from 1
const int ControlGUI::ktauOKCh               =  0; // output, IO26
const int ControlGUI::kBeamEnableCh          =  1; // output, IO27
const int ControlGUI::kDaqTriggerCh          =  2; // output, IO28
const int ControlGUI::kDaggerCh              =  3; // output, IO29
const int ControlGUI::kWestGateValveCh       =  4; // output, IO30
const int ControlGUI::kCleanerCh             =  5; // output, IO31
const int ControlGUI::kRoundhouseGateValveCh =  6; // output, IO32
const int ControlGUI::kButterflyCh           =  7; // output, IO33: starts MIDAS DAQ
const int ControlGUI::kTrapdoorCh1           =  8; // output, IO34: butterfly valve at input to cross assembly
const int ControlGUI::kTrapdoorCh2           =  9; // output, IO35
const int ControlGUI::kGiantCleanerCh        = 10; // output, IO36
const int ControlGUI::kSourceCtrlCh          = 11; // output, IO37: NEVER ACTUALLY IN USE
const int ControlGUI::kD2PumpOutCh           = 12; // output, IO38: NEVER ACTUALLY IN USE
const int ControlGUI::kHmGXCh                = 13; // input, IO39: H-GX from CCR, stretched to 1 s pulses via gate generator
const int ControlGUI::kGoLineCh              = 14; // input, IO40: NO LONGER IN USE
const int ControlGUI::kAbortCh               = 15; // input, IO41: NO LONGER IN USE

// Change these if necessary to get the logic sense correct:
//   1 ("true") means output HIGH (+5V).
const bool ControlGUI::ktauOK                   = 1;
const bool ControlGUI::kBeamOn                  = 1; // Beam veto on
const bool ControlGUI::kDaqRunning              = 1;
const bool ControlGUI::kDaggerPulse             = 1; // TODO: check this
const bool ControlGUI::kWestGateValveOpen       = 1;
const bool ControlGUI::kCleanerDown             = 1;
const bool ControlGUI::kRoundhouseGateValveOpen = 1;
const bool ControlGUI::kButterflyClosed         = 1;
const bool ControlGUI::kGiantCleanerDownOrPulse = 1; // TODO: check this
const bool ControlGUI::kPumpOutPortOpen         = 1; // NEVER ACTUALLY IN USE
const bool ControlGUI::kSourceCtrlOn            = 1; // NEVER ACTUALLY IN USE
const bool ControlGUI::kHmGXSignal              = 1;
const bool ControlGUI::kGoLineGood              = 1; // NO LONGER IN USE
const bool ControlGUI::kAbortSignal             = 1; // NEVER ACTUALLY IN USE


// Trapdoor options: must be 0,1,2,3
// More significant bit is sent to kTrapdoorCh1
// Less significant bit is sent to kTrapdoorCh2
//   1 ("true") means output HIGH (+24V)
const int ControlGUI::kTrapdoorNone = 0;
const int ControlGUI::kTrapdoorHold = 1;
const int ControlGUI::kTrapdoorDump = 2;
const int ControlGUI::kTrapdoorFill = 3;

const int ControlGUI::kPulseTimeMS   = 30; // 30 ms
const int ControlGUI::kBlockTimeMS   = 1000; // 1000 ms
#if GCFULLACTUATORINUSE
const int ControlGUI::kPulseChannels = 1<<kTrapdoorCh1 | 1<<kTrapdoorCh2 | 1<<kDaggerCh | 1<<kGiantCleanerCh;
const int ControlGUI::kPulseLow      = (kTrapdoorNone/2)<<kTrapdoorCh1 | (kTrapdoorNone%2)<<kTrapdoorCh2 | (!kDaggerPulse)<<kDaggerCh | (!kGiantCleanerDownOrPulse)<<kGiantCleanerCh;
#else
const int ControlGUI::kPulseChannels = 1<<kTrapdoorCh1 | 1<<kTrapdoorCh2 | 1<<kDaggerCh;
const int ControlGUI::kPulseLow      = (kTrapdoorNone/2)<<kTrapdoorCh1 | (kTrapdoorNone%2)<<kTrapdoorCh2 | (!kDaggerPulse)<<kDaggerCh;
#endif

// Lead/lags for enabling/disabling nEDM
const double ControlGUI::ktauOKLag  = 50.0; // 50 s, wait to turn on nEDM after fill
const double ControlGUI::ktauOKLead = 70.0; // 70 s, turn off nEDM before counting

ControlGUI::ControlGUI(QWidget *parent) : QMainWindow(parent),ui(new Ui::ControlGUI)
{   
	int nbytes;
	
	sync_count = 0; // init, see header file for more info
	
    // hack so that you can set up the GUI buttons/values without triggering any movement
    // will set false at end of this method
    fpga_run_in_progress = true;
    
    ui->setupUi(this);

    override_list=false;
    // add a progress indicator to the timing plot:
    progressLine = new QCPItemLine(ui->plot);
    ui->plot->addItem(progressLine);
    progressLine->start->setCoords(0., -1.);
    progressLine->end->setCoords(0., 7.);
    progressLine->setVisible(false);

    ui->loadfileNumber_button->setEnabled(false);
    ui->abortButton->setEnabled(false);
    //----------------------------------------------------------
    // Set all the timers
    this->InitTimers();
    //----------------------------------------------------------
    // Connect the GUI buttons to method Slots
    this->ConnectSlots();
    //----------------------------------------------------------
    // Do something with the Database
    this->get_midas_odb();

    // Set up the ZeroMQ connections to the servers
    context = zmq_ctx_new();
    socket_dagger = zmq_socket(context, ZMQ_REQ);
    socket_trapdoor = zmq_socket(context, ZMQ_REQ);
    socket_daq = zmq_socket(context,ZMQ_REQ);
    socket_fpga = zmq_socket(context,ZMQ_REQ);
#if FILLCHECK
    socket_fillCheck = zmq_socket(context,ZMQ_SUB);
#endif
#if GCFULLACTUATORINUSE
    socket_giantCleaner = zmq_socket(context, ZMQ_REQ);
#endif

    int timeout = 10000;
    zmq_setsockopt(socket_daq   ,ZMQ_RCVTIMEO,&timeout, sizeof (timeout));
#if FILLCHECK
    int fillCheck_timeout = 0;
    zmq_setsockopt(socket_fillCheck, ZMQ_RCVTIMEO, &fillCheck_timeout, sizeof(fillCheck_timeout));
#endif

    int rc = zmq_connect(socket_dagger, "ipc:///tmp/zeromq_dagger");
    assert(rc == 0);
    rc = zmq_connect(socket_trapdoor, "ipc:///tmp/zeromq_trapdoor");
    assert(rc == 0);
    rc = zmq_connect(socket_daq, "ipc:///tmp/zeromq_daq");
    assert(rc == 0);
    rc = zmq_connect(socket_fpga, "ipc:///tmp/zeromq_fpga");
    assert(rc == 0);
#if FILLCHECK
    rc = zmq_connect(socket_fillCheck, "ipc:///tmp/zeromq_fillCheck");
    assert(rc == 0);
#endif
#if GCFULLACTUATORINUSE
    rc = zmq_connect(socket_giantCleaner, "ipc:///tmp/zeromq_giantCleaner");
    assert(rc == 0);
#endif

    printf("Getting dagger position from server...");
    fflush(stdout);
    get_zdagger();   // update GUI based on present dagger height
    printf("Done.\n");
#if GCFULLACTUATORINUSE
    printf("Getting giant cleaner position from server...");
    fflush(stdout);
    get_zGiantCleaner();   // update GUI based on present cleaner height
    printf("Done.\n");
#endif
    printf("Getting trapdoor state from server...");
    fflush(stdout);
    get_trapdoor_state(); // update GUI based on present trapdoor state
    printf("Done.\n");
    fflush(stdout);
    printf("Getting FPGA params from server...");
    fflush(stdout);
    get_fpga_params(); // update GUI based on present FPGA state
    printf("Done.\n");
    fflush(stdout);

    ui->beamEnableRadio->setChecked(1<<kBeamEnableCh & (kBeamOn ? fpga_current_state : ~fpga_current_state));
    ui->westGateValveRadio->setChecked(1<<kWestGateValveCh & (kWestGateValveOpen ? fpga_current_state : ~fpga_current_state));
    ui->roundhouseGateValveRadio->setChecked(1<<kRoundhouseGateValveCh & (kRoundhouseGateValveOpen ? fpga_current_state : ~fpga_current_state));
    ui->cleanerRadio->setChecked(1<<kCleanerCh & (kCleanerDown ? fpga_current_state : ~fpga_current_state));
    ui->butterflyRadio->setChecked(1<<kButterflyCh & (kButterflyClosed ? fpga_current_state : ~fpga_current_state));
    ui->nEDMGVButton->setChecked(1<<ktauOKCh & (ktauOK ? fpga_current_state : ~fpga_current_state));
    ui->giantCleanerRadio->setChecked(1<<kGiantCleanerCh & (kGiantCleanerDownOrPulse ? fpga_current_state : ~fpga_current_state));   
    
    fpga_run_in_progress = false;

    QList<FpgaState> states;
    states.append(FpgaState(0,0,fpga_current_state));
    something_toggled(states);
    ui->textBrowser->append("Synched FPGA state with Slow Control state.");
    
#if GCFULLACTUATORINUSE && !TESTGIANTCLEANER
    ui->textBrowser->append("Trying to clear Giant Cleaner queue.");
    zmq_send(socket_giantCleaner, "ABRT", 4, 0);
    nbytes = zmq_recv(socket_giantCleaner, giantCleaner_recv_buffer, giantCleaner_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    giantCleaner_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString giantCleaner_str(giantCleaner_recv_buffer);
    if(giantCleaner_str=="ABORTED") ui->textBrowser->append("Giant Cleaner queue emptied.");
    else                            ui->textBrowser->append("WARNING! Giant Cleaner could not empty its queue.");
#endif

#if !TESTDAGGER
    ui->textBrowser->append("Trying to clear Dagger queue.");
    zmq_send(socket_dagger, "ABRT", 4, 0);
    nbytes = zmq_recv(socket_dagger, dagger_recv_buffer, dagger_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    dagger_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString dagger_str(dagger_recv_buffer);
    if(dagger_str=="ABORTED") ui->textBrowser->append("Dagger queue emptied.");
    else                      ui->textBrowser->append("WARNING! Dagger could not empty its queue.");
#endif

#if GCFULLACTUATORINUSE
    ui->giantCleanerRadio->setEnabled(false);
#else
    ui->giantCleanerPushButton->setEnabled(false);
    ui->giantCleanerSpinBox->setEnabled(false);
#endif
}

ControlGUI::~ControlGUI()
{
    delete ui;
}

// FPGA helper method
void ControlGUI::convert_time_to_ticks(double time, uint32_t &ticks0, uint32_t &ticks1)
{
    uint64_t total_ticks = (uint64_t)(time*fpga_timer_freq + 0.5);
    ticks1 = (uint32_t)(total_ticks/fpga_max_ticks);
    ticks0 = (uint32_t)(total_ticks - ticks1*fpga_max_ticks);
}

//FPGA helper method
void ControlGUI::set_nth_bit(uint16_t &data, uint16_t n, bool high_or_low)
{
    if(high_or_low) data |=   1<<n;
    else            data &= ~(1<<n);
}

void ControlGUI::get_zdagger()
{
double val;
#if !TESTDAGGER
    zmq_send(socket_dagger, "GET Z", 5, 0);
    int nbytes = zmq_recv(socket_dagger, dagger_recv_buffer, dagger_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    dagger_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string

    QString zdagger_str(dagger_recv_buffer);
    QStringList fields = zdagger_str.split(" ");

    if(fields.size() != 2) {
        ui->textBrowser->append(QString("Invalid reply from dagger server: ") + zdagger_str);
    }

    bool ok = false;
    val = fields[1].toDouble(&ok);

    if(!ok) {
        ui->textBrowser->append(QString("Problem getting dagger position from server: ") + zdagger_str);
    }
    ui->textBrowser->append(QString("zdagger = %1").arg(val));
#else
    val=380000.0;
    ui->textBrowser->append(QString("TEST: zdagger = %1").arg(val));
#endif
    ui->daggerSpinBox->setValue(val);
}

void ControlGUI::get_zGiantCleaner()
{
#if !GCFULLACTUATORINUSE
    ui->textBrowser->append(QString("Full Giant Cleaner actuator not installed."));
    return;
#endif

double val;
#if !TESTGIANTCLEANER
    zmq_send(socket_giantCleaner, "GET Z", 5, 0);
    int nbytes = zmq_recv(socket_giantCleaner, giantCleaner_recv_buffer, giantCleaner_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    giantCleaner_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString zGiantCleaner_str(giantCleaner_recv_buffer);
    QStringList fields = zGiantCleaner_str.split(" ");
    if(fields.size() != 2) {
        ui->textBrowser->append(QString("Invalid reply from giant cleaner server: ") + zGiantCleaner_str);
    }
    bool ok = false;
    val = fields[1].toDouble(&ok);
    if(!ok) {
        ui->textBrowser->append(QString("Problem getting giant cleaner position from server: ") + zGiantCleaner_str);
    }
    ui->textBrowser->append(QString("zGiantCleaner = %1").arg(val));
#else
    val = 100000.0;
    ui->textBrowser->append(QString("TEST: zGiantCleaner = %1").arg(val));
#endif
    ui->giantCleanerSpinBox->setValue(val);
}

void ControlGUI::get_fpga_params()
{
#if !TESTFPGA
    QString cmd = QString::number(kPulseChannels) + QString(" ") + QString::number(kPulseLow) + QString(" ") + QString::number(kPulseTimeMS) + QString(" ") + QString::number(kBlockTimeMS);
    zmq_send(socket_fpga, cmd.toLocal8Bit().data(), cmd.size(), 0);
    int nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    ui->textBrowser->append(QString("sent pulse info to FPGA"));
#else
    ui->textBrowser->append(QString("TEST: sent pulse info to FPGA"));
#endif

    double val1,val2,val3,val4;
#if !TESTFPGA
    zmq_send(socket_fpga, "GET PARAMS", 10, 0);
    nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString fpga_params_str(fpga_recv_buffer);
    QStringList fields = fpga_params_str.split(" ");
    if(fields.size()!=8 || fields.at(0)!="MAX_TICKS" || fields.at(2)!="TIMER_FREQ" || fields.at(4)!="PATTERN_LENGTH" || fields.at(6)!="INITIAL_STATE") ui->textBrowser->append(QString("Invalid reply from fpga server: ") + fpga_params_str);
    
    bool ok1=false, ok2=false, ok3=false, ok4=false;
    val1 = fields[1].toDouble(&ok1);
    val2 = fields[3].toDouble(&ok2);
    val3 = fields[5].toDouble(&ok3);
    val4 = fields[7].toDouble(&ok4);
    if(!(ok1&&ok2&&ok3&&ok4)) ui->textBrowser->append(QString("Problem getting fpga params from server: ") + fpga_params_str);
#else
    val1=4294967296.0;
    val2=83333333.0;
    val3=13.0;
    val4=1186.0;
#endif

    fpga_max_ticks     = (uint64_t)val1;
    fpga_timer_freq    = (uint32_t)val2;
    fpga_output_length = (int)val3;
    fpga_initial_state = (uint16_t)val4;
    fpga_current_state = fpga_initial_state;
    
#if !TESTFPGA
    ui->textBrowser->append(QString("Received timing params from FPGA."));
#else
    ui->textBrowser->append(QString("TEST: Received timing params from FPGA."));
#endif
}

void ControlGUI::reset_dagger_velacceldecel()
{
#if !TESTDAGGER
    zmq_send(socket_dagger, "RESET VEL", 9, 0);
    int nbytes = zmq_recv(socket_dagger, dagger_recv_buffer, dagger_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    dagger_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString dagger_str(dagger_recv_buffer);
    ui->textBrowser->append(QString("Reset dagger vel.") + dagger_str);
#else
    ui->textBrowser->append(QString("TEST: Resetting Dagger Velocity and Accel"));
#endif
}

void ControlGUI::reset_giantCleaner_velacceldecel()
{
#if !GCFULLACTUATORINUSE
    ui->textBrowser->append(QString("Full Giant Cleaner actuator not installed."));
    return;
#endif

#if !TESTGIANTCLEANER
    zmq_send(socket_giantCleaner, "RESET VEL", 9, 0);
    int nbytes = zmq_recv(socket_giantCleaner, giantCleaner_recv_buffer, giantCleaner_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    giantCleaner_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString giantCleaner_str(giantCleaner_recv_buffer);
    ui->textBrowser->append(QString("Reset giant cleaner vel.") + giantCleaner_str);
#else
    ui->textBrowser->append(QString("TEST: Resetting GiantCleaner Velocity and Accel"));
#endif
}

void ControlGUI::get_trapdoor_state()
{
int i_item;
#if !TESTTRAPDOOR
    zmq_send(socket_trapdoor, "GET STATE", 9, 0);
    int nbytes = zmq_recv(socket_trapdoor, trapdoor_recv_buffer, trapdoor_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    trapdoor_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString trapdoor_str(trapdoor_recv_buffer);
    QStringList fields = trapdoor_str.split(" ");
    if(fields.size() != 3) {
        ui->textBrowser->append(QString("Invalid reply from trapdoor server: ") + trapdoor_str);
    }
    QString state_str = fields[2];
#else
    QString state_str("DONT KNOW");
#endif
    i_item = ui->trapdoorComboBox->findText(state_str);
    if(i_item < 0) {
        ui->textBrowser->append("Problem getting trapdoor state from server");
    } else {
        ui->trapdoorComboBox->setCurrentIndex(i_item);
    }
}

void ControlGUI::openFile_clicked()
{
    ui->openFileButton->setEnabled(false);
    QString filename = QFileDialog::getOpenFileName(this, tr("Open run file"), kControlDir + "RunFiles");

    if(filename == "") {
        ui->textBrowser->append("File open cancelled.");
	    ui->openFileButton->setEnabled(true);
        return;
    }

    ui->filenameLineEdit->setText(filename);

    runFileNames.clear();

    QFile f(filename);
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&f);

    while (!in.atEnd())
    {
        QString line = in.readLine();
        // remove comment text
        int icomment = line.indexOf("#", 0);
        if(icomment >= 0) {
            line.truncate(icomment);
        }
        // key=value pairs should be separated by ','
        QStringList listLine = line.split(",",QString::SkipEmptyParts);
        // check if there's at least one '=' in the list
        if(listLine.size() == 0) continue;
        if(listLine[0].indexOf("=",0) < 0) continue;

        QStringList keyval=listLine[0].split("=");
        assert(keyval.size() == 2);
        QString firstkey = keyval[0].simplified();
        if(firstkey == "FILE") {
            int ieq = line.indexOf("=");
            if(ieq > 0) {
                runFileNames.append(line.remove(0,ieq+1).trimmed());
            }
            continue;
        }
    } // end while loop over lines
    f.close();

    // if no FILE= key-val pairs were found, this must be a regular parameter file.
    if(runFileNames.size() < 1) runFileNames.append(filename);

    ui->textBrowser->append(runFileNames.join("\n"));
    if(runFileNames.size()>1)ui->loadfileNumber_button->setEnabled(true);
    ui->listnumber_spinbox->setRange(0,(int)runFileNames.size()-1);
    updateTimingPattern(runFileNames[0]);
    
    ui->openFileButton->setEnabled(true);
}

bool ControlGUI::updateTimingPattern(QString filename)
{
    beamEnableStates.clear();
    cleanerStates.clear();
    westGateValveStates.clear();
    roundhouseGateValveStates.clear();
    trapdoorStates.clear();
    daggerPositions.clear();
    giantCleanerPositions.clear();
    butterflyStates.clear();
    nEDMstates.clear();
    PumpOutStates.clear();
    fpgaActions.clear();
    fpgaStates.clear();
    daggerQueue.clear();
    giantCleanerQueue.clear();
    
    // FPGA timing parameters
    uint32_t ticks0,ticks1;

    // Start with nEDM suppressed, DAQ triggered to start
    nEDMstates.append(ControlEvent<bool>(0,true,0));
    fpgaActions.append(FpgaAction(0,0,ktauOKCh,!ktauOK));
    fpgaActions.append(FpgaAction(0,0,kDaqTriggerCh,kDaqRunning));
    
    // variables to track dagger movement in the simple dagger cleaning
    bool   dagger_up          = false;
    double initial_dagger_pos = 0;
    bool   holding            = false;
    bool   counting           = false;
    clean_time                = 0;
    count_time                = 0;
    double clean_itime        = 0.0;
    double count_itime        = 0.0;
    deltat                    = 0;
    double itime              = 0.0;
    
    QFile f(filename);
    f.open(QIODevice::ReadOnly | QIODevice::Text);

    if(!f.isOpen()) {
        ui->textBrowser->append(QString("File not found: ") + filename);
        return false;
    }
    
    std::vector<int> all_times;
    
    QTextStream in(&f);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        // remove comment text
        int icomment = line.indexOf("#", 0);
        if(icomment >= 0) {
            line.truncate(icomment);
        }
        // key=value pairs should be separated by ','
        QStringList listLine = line.split(",",QString::SkipEmptyParts);
        // check if there's at least one '=' in the list
        if(listLine.size() == 0) continue;
        if(listLine[0].indexOf("=",0) < 0) continue;
        double t_line = 0; // elapsed time of this line
        QStringList keyval=listLine[0].split("=");
        assert(keyval.size() == 2);
        QString firstkey = keyval[0].simplified();

        if(firstkey == "T") {
            t_line = keyval[1].toDouble();
            itime  = t_line;
            if(holding) {
                if(!counting) {
                    deltat = (t_line - clean_itime)*(1.0-factor);
                }
                t_line += deltat;
            }
            all_times.push_back(t_line);
            
        #if DEBUG_BLIND
            printf(" State of Dagger up : %d at time %f \n",dagger_up,t_line);
        #endif
        } else if(firstkey == "COMMENT") {
            int ieq = line.indexOf("=");
            if(ieq > 0) {
                ui->commentLineEdit->setText(line.remove(0,ieq+1).trimmed());
            } else {
                ui->textBrowser->append("Malformed COMMENT line");
            }
            continue;
        } else {
            ui->textBrowser->append("Error parsing file: first key-value pair per non-COMMENT line must be T=[time]");
            continue;
        }
        
        for(int ikeyval=1; ikeyval < listLine.size(); ikeyval++) {
            QStringList keyval = listLine[ikeyval].split("=");
            assert(keyval.size() >= 2);
            QString key = keyval[0].simplified(); // remove whitespace
            QString valstr = keyval[1].simplified(); // remove whitespace
            
            convert_time_to_ticks(t_line,ticks0,ticks1);
            
            // FPGA handles the pulsing internally so don't need to unset the pulses
            
            if(key == "TRAPDOOR") {
                trapdoorStates.append(ControlEvent<QString>(t_line, valstr,itime));
                if(valstr=="FILL"){
                    fpgaActions.append(FpgaAction(ticks0,ticks1,kTrapdoorCh1,kTrapdoorFill/2));
                    fpgaActions.append(FpgaAction(ticks0,ticks1,kTrapdoorCh2,kTrapdoorFill%2));
                }
                else if(valstr=="HOLD"){
                    fpgaActions.append(FpgaAction(ticks0,ticks1,kTrapdoorCh1,kTrapdoorHold/2));
                    fpgaActions.append(FpgaAction(ticks0,ticks1,kTrapdoorCh2,kTrapdoorHold%2));
                }
                else if(valstr=="DUMP"){
                    fpgaActions.append(FpgaAction(ticks0,ticks1,kTrapdoorCh1,kTrapdoorDump/2));
                    fpgaActions.append(FpgaAction(ticks0,ticks1,kTrapdoorCh2,kTrapdoorDump%2));
                }
                else{ // invalid command, do nothing to trapdoor
                    fpgaActions.append(FpgaAction(ticks0,ticks1,kTrapdoorCh1,kTrapdoorNone/2));
                    fpgaActions.append(FpgaAction(ticks0,ticks1,kTrapdoorCh2,kTrapdoorNone%2));
                }
            } else if(key == "DAGGER") {
                daggerPositions.append(ControlEvent<double>(t_line, valstr.toDouble(),itime));
                fpgaActions.append(FpgaAction(ticks0,ticks1,kDaggerCh,kDaggerPulse));
            #if DEBUG_BLIND
                printf("dagger current position and initial: %f  \t %f \n", valstr.toDouble(),initial_dagger_pos);
            #endif
                if( !dagger_up && initial_dagger_pos == 0){
                    initial_dagger_pos = valstr.toDouble();
                }else if( !dagger_up && initial_dagger_pos != 0 && valstr.toDouble() > initial_dagger_pos){
                    dagger_up      = true;
                    dagger_up_time = t_line;
                }
                daggerQueue.append(QString("QUEUE Z ") + QString::number(int(valstr.toDouble())));
            } else if(key == "GATEVALVE") { // naming artifact from before we had multiple gate valves TODO
                bool state = false; // closed
                if(valstr == "OPEN") state = true;
            #if DEBUG_BLIND
                printf("WestGateValve timing: %f  state : %d \n",t_line,state);
            #endif
                westGateValveStates.append(ControlEvent<bool>(t_line, state,itime));
                fpgaActions.append(FpgaAction(ticks0,ticks1,kWestGateValveCh,state ? kWestGateValveOpen : !kWestGateValveOpen));
                roundhouseGateValveStates.append(ControlEvent<bool>(t_line, state,itime)); // TODO: seperate out when you edit RunFiles
                fpgaActions.append(FpgaAction(ticks0,ticks1,kRoundhouseGateValveCh,state ? kRoundhouseGateValveOpen : !kRoundhouseGateValveOpen));
            } else if(key == "BEAM") {
                bool state = false; // veto off == protons on target
                if(valstr == "ON") state = true;
                beamEnableStates.append(ControlEvent<bool>(t_line, state,itime));
                fpgaActions.append(FpgaAction(ticks0,ticks1,kBeamEnableCh,state ? kBeamOn : !kBeamOn));
            } else if( key == "D2PORT"){
                bool state = false;
                if(valstr == "OPEN") state = true;
                PumpOutStates.append(ControlEvent<bool>(t_line,state,itime));
                fpgaActions.append(FpgaAction(ticks0,ticks1,kD2PumpOutCh,state ? kPumpOutPortOpen : !kPumpOutPortOpen));
            } else if(key == "BUTTERFLY") {
                bool state = false; // butterfly open
                if(valstr == "CLOSED") state = true;
            #if DEBUG_BLIND
                printf("Butterfly timing: %f state : %d \n",t_line,state);
            #endif
                butterflyStates.append(ControlEvent<bool>(t_line, state,itime));
                fpgaActions.append(FpgaAction(ticks0,ticks1,kButterflyCh,state ? kButterflyClosed : !kButterflyClosed));
            } else if(key == "CLEANER") {
                bool state = false; // cleaner up
            #if DEBUG_BLIND
                printf("Cleaner timing: %f  state : %d \n",t_line,state);
            #endif
                if(valstr == "DOWN") state = true;
                cleanerStates.append(ControlEvent<bool>(t_line, state,itime));
                fpgaActions.append(FpgaAction(ticks0,ticks1,kCleanerCh,state ? kCleanerDown : !kCleanerDown));
            } else if(key == "GIANTCLEANER") {
                giantCleanerPositions.append(ControlEvent<double>(t_line, valstr.toDouble(),itime));
            #if GCFULLACTUATORINUSE
                fpgaActions.append(FpgaAction(ticks0,ticks1,kGiantCleanerCh,kGiantCleanerDownOrPulse));
                giantCleanerQueue.append(QString("QUEUE Z ") + QString::number(int(valstr.toDouble())));
            #else
                if (valstr.toDouble()<=100000.0) fpgaActions.append(FpgaAction(ticks0,ticks1,kGiantCleanerCh,kGiantCleanerDownOrPulse));
                else                             fpgaActions.append(FpgaAction(ticks0,ticks1,kGiantCleanerCh,!kGiantCleanerDownOrPulse));
            #endif
            } else if(key == "SOURCE"){
                bool state = false;
                if(valstr == "ON") state = true;
                SourceCtrlStates.append(ControlEvent<bool>(t_line,state,itime));
                fpgaActions.append(FpgaAction(ticks0,ticks1,kSourceCtrlCh,state ? kSourceCtrlOn : !kSourceCtrlOn));
            } else if(key == "HOLD"){
                holding = true;
                clean_time = t_line;
                clean_itime = itime;
            } else if(key == "COUNT"){
		        counting = true;
                count_time = t_line;
                count_itime = itime;
                // have to check to see if there is enough time for nEDM to turn on
                if(clean_time+ktauOKLag<count_time-ktauOKLead){
                    nEDMstates.append(ControlEvent<bool>(clean_time+ktauOKLag,false,clean_itime+ktauOKLag));
                    convert_time_to_ticks(clean_time+ktauOKLag,ticks0,ticks1);
                    fpgaActions.append(FpgaAction(ticks0,ticks1,ktauOKCh,ktauOK));
                    
                    nEDMstates.append(ControlEvent<bool>(count_time-ktauOKLead,true,count_itime-ktauOKLead));
                    convert_time_to_ticks(count_time-ktauOKLead,ticks0,ticks1);
                    fpgaActions.append(FpgaAction(ticks0,ticks1,ktauOKCh,!ktauOK));
                }
            }

        } // end for ikeyval

    }
    f.close();
    
    // display run length
    double last_time = itime;
    ui->cyclePeriodLCD->display((int)last_time);
    
    last_check_time=0.0;
    for (int i=all_times.size()-1;i>0;i--){
        if(all_times.at(i)-kBlockTimeMS/1000.0>all_times.at(i-1)) {
            last_check_time = all_times.at(i)-kBlockTimeMS/1000.0;
            break;
        }
    }
    
    uint16_t fpga_working_state = fpga_current_state;
    // uses the fpgaActions to make the list fpgaStates
    while(fpgaActions.size()>0){
        // find min time in remaining actions
        uint32_t min0 = fpgaActions.at(0).ticks0();
        uint32_t min1 = fpgaActions.at(0).ticks1();
        for(int iAction=1;iAction<fpgaActions.size();iAction++){
            if(fpgaActions.at(iAction).ticks1()<min1 || (fpgaActions.at(iAction).ticks1()==min1 && fpgaActions.at(iAction).ticks0()<min0)){
                min0 = fpgaActions.at(iAction).ticks0();
                min1 = fpgaActions.at(iAction).ticks1();
            }
        }
        
        // perform all actions at that time
        for(int iAction=0;iAction<fpgaActions.size();){ // don't iAction++ here, in the loop either iAction++ or remove an element
            if(min0!=fpgaActions.at(iAction).ticks0() || min1!=fpgaActions.at(iAction).ticks1()) iAction++;
            else{
                set_nth_bit(fpga_working_state,fpgaActions.at(iAction).fpga_channel(),fpgaActions.at(iAction).channel_state());
                fpgaActions.removeAt(iAction);
            }
        }
        
        // set the input bits to 0 (shouldn't matter but just to be safe)
        for(int iBit=fpga_output_length;iBit<8*sizeof(fpga_working_state);iBit++) set_nth_bit(fpga_working_state,iBit,0);
        
        // if done with all of the actions, set DAQ trigger low
        if(fpgaActions.size()==0) set_nth_bit(fpga_working_state,kDaqTriggerCh,!kDaqRunning);

        // save this state
        fpgaStates.append(FpgaState(min0,min1,fpga_working_state));
        
        // reset trapdoor working state to NONE, giant cleaner and dagger pulses to !(timing pulse)
        // otherwise a pulse is sent at any transition regardless of if the actuator moves
        set_nth_bit(fpga_working_state,kTrapdoorCh1,kTrapdoorNone/2);
        set_nth_bit(fpga_working_state,kTrapdoorCh2,kTrapdoorNone/2);
        set_nth_bit(fpga_working_state,kDaggerCh,!kDaggerPulse);
    #if GCFULLACTUATORINUSE
        set_nth_bit(fpga_working_state,kGiantCleanerCh,!kGiantCleanerDownOrPulse);
    #endif
    }

#if DEBUG_BLIND
    printf("cleaning time : %f and counting time %f\n",clean_time,count_time);
#endif

    updatePlot();
    ui->startButton->setEnabled(true);
    return true;
}

double get_double_rep(bool st)
{
    return st ? 1. : 0.;
}

double get_double_rep(double st)
{
    return st;
}

// FIXME: this is only for the TRAPDOOR. Should really make a trapdoor typedef.
double get_double_rep(QString st)
{
    if((st == "HOLD") || (st == "UP")) {
        return 1.;
    } else {
        return 0.;
    }
}

template<typename statetype> void make_plot_arrays(QList<ControlEvent<statetype> > events,
                                                   double offset, double amplitude, double xmax,
                                                   QVector<double> &x, QVector<double> &y) {
    x.clear();
    y.clear();

    if(events.size() < 1) return;

    x.append(events.at(0).bltime());
    statetype state = events.at(0).state();
    y.append(offset + amplitude*get_double_rep(state));
    for(int i=1; i < events.size(); i++) {
        double bltime = events.at(i).bltime();
        statetype newstate = events.at(i).state();
        x.append(bltime - 0.01);
        y.append(offset + amplitude*get_double_rep(state));
        x.append(bltime);
        y.append(offset + amplitude*get_double_rep(newstate));
        state = newstate;
    }
    if(x.last() < xmax) {
        x.append(xmax);
        y.append(y.last());
    }
}

void ControlGUI::updatePlot()
{
    ui->plot->clearPlottables();
    ui->plot->legend->setVisible(true);

    double xmin = 0.;
    double xmax = ui->cyclePeriodLCD->value();

    ui->plot->xAxis->setRange(xmin, xmax);
    ui->plot->yAxis->setRange(-0.1, 10.0);

    QCPGraph *gr;

    // Add states to the graph.
    addToGraph("Beam On",beamEnableStates,6,0.9,xmax,Qt::black,gr);
    addToGraph("Roundhouse Gate Valve Open",roundhouseGateValveStates,5,0.9,xmax,Qt::darkGreen,gr);
    addToGraph("West Gate Valve Open",westGateValveStates,4,0.9,xmax,Qt::blue,gr);
    addToGraph("Giant Cleaner Position (arb)",giantCleanerPositions,3,1./10387500.*0.9,xmax,Qt::cyan,gr);
    addToGraph("Cleaner Down",cleanerStates,2,0.9,xmax,Qt::green,gr);
    addToGraph("Trap Door Closed",trapdoorStates,1,0.9,xmax,Qt::red,gr);
    addToGraph("Dagger Position (arb)",daggerPositions, 0 ,1./550000,xmax,Qt::magenta,gr);

    ui->plot->replot();
}

template<typename statetype> void ControlGUI::addToGraph(const char* gName,QList<ControlEvent<statetype> > states,double offset
                                                         ,double scale, double xmax,const QColor color, QCPGraph *gGraph)
{
    QVector<double> x;
    QVector<double> y;

    gGraph = ui->plot->addGraph();
    gGraph->setPen(QPen(color));
    gGraph->setName(gName);
    make_plot_arrays(states,offset, scale, xmax, x, y);
    gGraph->setData(x, y);
}

void ControlGUI::something_toggled(QList<FpgaState> states)
{   
    enableChannelButtons(false);
    QTimer::singleShot(2000, [this]() {
        enableChannelButtons(true);
    });
       
#if !TESTFPGA
    bool result = load_fpga(states);
    if(!result){
        ui->textBrowser->append("FPGA couldn't do the toggle request.");
        ui->textBrowser->append("WARNING!!! This is probably because you clicked some button multiple times in rapid successsion. DON'T DO THAT!");
        ui->textBrowser->append("The current FPGA state may be out of synch with the Slow Control. Restart Slow Control and FPGA server.");
        return;
    }
    zmq_send(socket_fpga,"START FPGA NOW",14,0);
    int nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    zmq_send(socket_fpga, "SLOW CONTROL AT END OF RUN", 26, 0);
    nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString fpga_str(fpga_recv_buffer);
    if(fpga_str=="RUN ABORTED")   ui->textBrowser->append("FPGA reports that the toggle was aborted.");
    else if(fpga_str=="RUN GOOD") ui->textBrowser->append("FPGA reports that it toggled something.");
    else                          ui->textBrowser->append("FPGA reports that it had trouble toggling something.");
#else
    ui->textBrowser->append("TEST: FPGA toggled something.");
#endif
}

void ControlGUI::nedmEnable_toggled(bool checked)
{
    set_nth_bit(fpga_current_state,ktauOKCh,!checked^ktauOK);
    if(fpga_run_in_progress) return;
    QList<FpgaState> states;
    states.append(FpgaState(0,0,fpga_current_state));
    something_toggled(states);
}

void ControlGUI::beamEnable_toggled(bool checked)
{
    set_nth_bit(fpga_current_state,kBeamEnableCh,!checked^kBeamOn);
    if(fpga_run_in_progress) return;
    QList<FpgaState> states;
    states.append(FpgaState(0,0,fpga_current_state));
    something_toggled(states);
}

void ControlGUI::westGateValve_toggled(bool checked)
{
    set_nth_bit(fpga_current_state,kWestGateValveCh,!checked^kWestGateValveOpen);
    if(fpga_run_in_progress) return;
    QList<FpgaState> states;
    states.append(FpgaState(0,0,fpga_current_state));
    something_toggled(states);
}

void ControlGUI::roundhouseGateValve_toggled(bool checked)
{
    set_nth_bit(fpga_current_state,kRoundhouseGateValveCh,!checked^kRoundhouseGateValveOpen);
    if(fpga_run_in_progress) return;
    QList<FpgaState> states;
    states.append(FpgaState(0,0,fpga_current_state));
    something_toggled(states);
}

void ControlGUI::cleaner_toggled(bool checked)
{
    set_nth_bit(fpga_current_state,kCleanerCh,!checked^kCleanerDown);
    if(fpga_run_in_progress) return;
    QList<FpgaState> states;
    states.append(FpgaState(0,0,fpga_current_state));
    something_toggled(states);
}

void ControlGUI::butterfly_toggled(bool checked)
{
    set_nth_bit(fpga_current_state,kButterflyCh,!checked^kButterflyClosed);
    if(fpga_run_in_progress) return;
    QList<FpgaState> states;
    states.append(FpgaState(0,0,fpga_current_state));
    something_toggled(states);
}

void ControlGUI::giantCleaner_toggled(bool checked)
{
    set_nth_bit(fpga_current_state,kGiantCleanerCh,!checked^kGiantCleanerDownOrPulse);
    if(fpga_run_in_progress) return;
    QList<FpgaState> states;
    states.append(FpgaState(0,0,fpga_current_state));
    something_toggled(states);
}

void ControlGUI::trapdoor_index_changed(int index)
{
    if(fpga_run_in_progress) return;
    QString valstr = QString(ui->trapdoorComboBox->itemText(index));

    int state = kTrapdoorNone;
    if(valstr=="FILL") state = kTrapdoorFill;
    else if(valstr=="HOLD") state = kTrapdoorHold;
    else if(valstr=="DUMP") state = kTrapdoorDump;
    else state = kTrapdoorNone;
#if !TESTFPGA
    ui->textBrowser->append(QString("Setting trapdoor to ") + valstr);
    QList<FpgaState> states;
    set_nth_bit(fpga_current_state,kTrapdoorCh1,state/2);
    set_nth_bit(fpga_current_state,kTrapdoorCh2,state%2);
    states.append(FpgaState(0,0,fpga_current_state));
    set_nth_bit(fpga_current_state,kTrapdoorCh1,kTrapdoorNone/2);
    set_nth_bit(fpga_current_state,kTrapdoorCh2,kTrapdoorNone%2);
    something_toggled(states);
#else
    ui->textBrowser->append(QString("TEST: Setting trapdoor to ") + valstr);
#endif
}

void ControlGUI::dagger_button_pushed()
{
    if(fpga_run_in_progress) return;
#if !TESTDAGGER
    QString cmd = QString("SET Z ") + QString::number(int(ui->daggerSpinBox->value()));
    zmq_send(socket_dagger, cmd.toLocal8Bit().data(), cmd.size(), 0);
    int nbytes = zmq_recv(socket_dagger, dagger_recv_buffer, dagger_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    dagger_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString zdagger_str(dagger_recv_buffer);
    QStringList fields = zdagger_str.split(" ");
    if((fields.size() != 2) || (fields.at(0) != "Z")) {
        ui->textBrowser->append(QString("Invalid reply from dagger server: ") + zdagger_str);
    }
    bool ok = false;
    double val = fields[1].toDouble(&ok);
    if(!ok) {
        ui->textBrowser->append(QString("Problem getting dagger position from server: ") + zdagger_str);
        ui->textBrowser->append(QString("z = %1").arg(val));
        ui->textBrowser->append(fields[0]);
        ui->textBrowser->append(fields[1]);
    }
    if(fabs(val - ui->daggerSpinBox->value()) > 2) {
        ui->textBrowser->append("Dagger not set to requested position.");
    }
#else
    ui->textBrowser->append(QString("TEST: Attempting to push dagger button."));
#endif
}

void ControlGUI::giantCleaner_button_pushed()
{
#if !GCFULLACTUATORINUSE
    ui->textBrowser->append(QString("Full Giant Cleaner actuator not installed."));
    return;
#endif

    if(fpga_run_in_progress) return;
#if !TESTGIANTCLEANER
    QString cmd = QString("SET Z ") + QString::number(int(ui->giantCleanerSpinBox->value()));
    zmq_send(socket_giantCleaner, cmd.toLocal8Bit().data(), cmd.size(), 0);
    int nbytes = zmq_recv(socket_giantCleaner, giantCleaner_recv_buffer, giantCleaner_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    giantCleaner_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString zGiantCleaner_str(giantCleaner_recv_buffer);
    QStringList fields = zGiantCleaner_str.split(" ");
    if((fields.size() != 2) || (fields.at(0) != "Z")) {
        ui->textBrowser->append(QString("Invalid reply from giant cleaner server: ") + zGiantCleaner_str);
    }
    bool ok = false;
    double val = fields[1].toDouble(&ok);
    if(!ok) {
        ui->textBrowser->append(QString("Problem getting giant cleaner position from server: ") + zGiantCleaner_str);
        ui->textBrowser->append(QString("z = %1").arg(val));
        ui->textBrowser->append(fields[0]);
        ui->textBrowser->append(fields[1]);
    }
    if(fabs(val - ui->giantCleanerSpinBox->value()) > 2) {
        ui->textBrowser->append("Giant cleaner not set to requested position.");
    }
#else
    ui->textBrowser->append(QString("TEST: Attempting to push giant cleaner button."));
#endif
}

void ControlGUI::enableChannelButtons(bool state)
{
    ui->beamEnableRadio->setEnabled(state);
    ui->westGateValveRadio->setEnabled(state);
    ui->roundhouseGateValveRadio->setEnabled(state);
    ui->cleanerRadio->setEnabled(state);
    ui->trapdoorComboBox->setEnabled(state);
    ui->daggerPushButton->setEnabled(state);
    ui->daggerSpinBox->setEnabled(state);
    ui->openFileButton->setEnabled(state);
    ui->nEDMGVButton->setEnabled(state);
    ui->butterflyRadio->setEnabled(state);
#if GCFULLACTUATORINUSE
    ui->giantCleanerPushButton->setEnabled(state);
    ui->giantCleanerRadio->setEnabled(false);
#else
    ui->giantCleanerRadio->setEnabled(state);
    ui->giantCleanerPushButton->setEnabled(false);
    ui->giantCleanerSpinBox->setEnabled(false);
#endif
}

void ControlGUI::startButton_clicked(bool firstcall, bool updatePattern)
{
	int nbytes;
	
    if(runFileNames.size() < 1) {
        ui->textBrowser->append("No file loaded.");
        return;
    }

    if(firstcall && !override_list) iStart = 0;

    if(iStart < runFileNames.size() && updatePattern) {
        if(updateTimingPattern(runFileNames.at(iStart))){
            ui->listnumber_spinbox->setValue(iStart); // update the list number
        } else {
            ui->textBrowser->append(QString("File %1 is incorrect").arg(runFileNames.at(iStart)));
            return;
        }
    } else {
        return;
    }

    enableChannelButtons(false);
    ui->startButton->setEnabled(false);

    progressLine->start->setCoords(0., -1.);
    progressLine->end->setCoords(0., 5.);
    progressLine->setVisible(true);

    ui->plot->replot(); // refresh to plot to include the progress line
    
    // Ask dagger_server.py to resend the velocity, acceleration, and deceleration
    // settings to the motor controller. This might help prevent an accident if
    // the motor controller was power cycled without restarting dagger_server.py.
    reset_dagger_velacceldecel();

    // Ask giantCleaner_server.py to resend the velocity, acceleration, and deceleration
    // settings to the motor controller. This might help prevent an accident if
    // the motor controller was power cycled without restarting giantCleaner_server.py.
    reset_giantCleaner_velacceldecel();

    bool can_fpga_load = load_fpga(fpgaStates);
    if(!can_fpga_load) {
        ui->textBrowser->append(QString("ERROR! FPGA wasn't able to load the run."));
        enableChannelButtons(true);
        ui->startButton->setEnabled(true);
        ui->abortButton->setEnabled(false);
        progressLine->setVisible(false);
        return;
    }
    
#if !TESTDAGGER
    ui->textBrowser->append("Trying to clear Dagger queue.");
    zmq_send(socket_dagger, "ABRT", 4, 0);
    nbytes = zmq_recv(socket_dagger, dagger_recv_buffer, dagger_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    dagger_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString dagger_str(dagger_recv_buffer);
    if(dagger_str=="ABORTED") ui->textBrowser->append("Dagger queue emptied.");
    else                      ui->textBrowser->append("WARNING! Dagger could not empty its queue.");
#endif
    for(int i_daggerQueue=0;i_daggerQueue<daggerQueue.size();i_daggerQueue++){
    #if !TESTDAGGER
        zmq_send(socket_dagger, daggerQueue.at(i_daggerQueue).toLocal8Bit().data(), daggerQueue.at(i_daggerQueue).size(), 0);
        inbytes = zmq_recv(socket_dagger, dagger_recv_buffer, dagger_recv_buffer_size-1, 0);
        assert(nbytes != -1);
        dagger_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
        QString zdagger_str(dagger_recv_buffer);
        QStringList fields = zdagger_str.split(" ");
        if((fields.size() != 3) || (fields.at(0) != "QUEUED") || (fields.at(1) != "Z")) {
            ui->textBrowser->append(QString("Invalid reply from dagger server: ") + zdagger_str);
        }
        bool ok = false;
        double val = fields[2].toDouble(&ok);
        if(!ok) {
            ui->textBrowser->append(QString("Problem getting queued dagger position from server: ") + zdagger_str);
            ui->textBrowser->append(QString("z = %1").arg(val));
            ui->textBrowser->append(fields[0]);
            ui->textBrowser->append(fields[1]);
            ui->textBrowser->append(fields[2]);
        }
    #else
        ui->textBrowser->append(QString("TEST: Attempting to queue dagger position."));
    #endif
    }

#if GCFULLACTUATORINUSE
    #if !TESTGIANTCLEANER
    ui->textBrowser->append("Trying to clear Giant Cleaner queue.");
    zmq_send(socket_giantCleaner, "ABRT", 4, 0);
    nbytes = zmq_recv(socket_giantCleaner, giantCleaner_recv_buffer, giantCleaner_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    giantCleaner_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString giantCleaner_str(giantCleaner_recv_buffer);
    if(giantCleaner_str=="ABORTED") ui->textBrowser->append("Giant Cleaner queue emptied.");
    else                            ui->textBrowser->append("WARNING! Giant Cleaner could not empty its queue.");
    
    for (int i_giantCleanerQueue=0;i_giantCleanerQueue<giantCleanerQueue.size();i_giantCleanerQueue++){
        zmq_send(socket_giantCleaner, giantCleanerQueue.at(i_giantCleanerQueue).toLocal8Bit().data(), giantCleanerQueue.at(i_giantCleanerQueue).size(), 0);
        int nbytes = zmq_recv(socket_giantCleaner, giantCleaner_recv_buffer, giantCleaner_recv_buffer_size-1, 0);
        assert(nbytes != -1);
        giantCleaner_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
        QString zGiantCleaner_str(giantCleaner_recv_buffer);
        QStringList fields = zGiantCleaner_str.split(" ");
        if((fields.size() != 3) || (fields.at(0) != "QUEUED") || (fields.at(1) != "Z")) {
            ui->textBrowser->append(QString("Invalid reply from giant cleaner server: ") + zGiantCleaner_str);
        }
        bool ok = false;
        double val = fields[2].toDouble(&ok);
        if(!ok) {
            ui->textBrowser->append(QString("Problem getting queued giant cleaner position from server: ") + zGiantCleaner_str);
            ui->textBrowser->append(QString("z = %1").arg(val));
            ui->textBrowser->append(fields[0]);
            ui->textBrowser->append(fields[1]);
            ui->textBrowser->append(fields[2]);
        }
    }
    #else
        ui->textBrowser->append(QString("TEST: Attempting to queue giant cleaner button."));
    #endif
#endif
    
    ui->textBrowser->append(QString("Waiting for start conditions..."));
    waitForGoThenStart(true);
}

// have to pass in a FpgaState list so we can reuse this for length-one (used to move FPGA outside of a run)
// returns true if FPGA is primed and ready to fire, just waiting final command
// returns false if FPGA is currently running, or FPGA loaded incorrectly and now FPGA has been cleaered
bool ControlGUI::load_fpga(QList<FpgaState> states)
{
    if(fpga_run_in_progress) return false;
#if !TESTFPGA
    bool good_server_load = true;    
    
    QString cmd = QString("LOAD RUNFILE ") + QString::number((int)(states.size()));
    zmq_send(socket_fpga, cmd.toLocal8Bit().data(), cmd.size(), 0);
    int nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString fpga_str(fpga_recv_buffer);
    QStringList fields = fpga_str.split(" ");
    if((fields.size() != 2) || (fields.at(0) != "N")) {
        ui->textBrowser->append(QString("Invalid reply from FPGA server: ") + fpga_str);
    }
    bool ok = false;
    int n = (int)(fields[1].toDouble(&ok));
    if(!ok) {
        good_server_load = false;
        ui->textBrowser->append(QString("Problem getting n from FPGA server: ") + fpga_str);
        ui->textBrowser->append(QString("n = %1").arg(n));
        ui->textBrowser->append(fields[0]);
        ui->textBrowser->append(fields[1]);
    }
    
    if(n!=(int)(states.size())){
        good_server_load = false;
        zmq_send(socket_fpga,"BAD N",5,0);
        nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
        assert(nbytes != -1);
        fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
        ui->textBrowser->append("FPGA server heard the wrong n:" + QString(fpga_recv_buffer));
    }else{
        zmq_send(socket_fpga,"GOOD N",6,0);
        nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
        for(int iState=0;iState<states.size();iState++){
            cmd = QString("TICKS0 ") + QString::number(states.at(iState).ticks0()) + QString(" TICKS1 ") + QString::number(states.at(iState).ticks1()) + QString(" STATE ") + QString::number(states.at(iState).fpga_state());
            zmq_send(socket_fpga, cmd.toLocal8Bit().data(), cmd.size(), 0);
            nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
            assert(nbytes != -1);
            fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
            QString valFpga_str(fpga_recv_buffer);
            QStringList fields = valFpga_str.split(" ");
            if((fields.size() != 6) || (fields.at(0) != "TICKS0") || (fields.at(2) != "TICKS1") || (fields.at(4) != "STATE")) {
                ui->textBrowser->append(QString("Invalid reply from FPGA server: ") + valFpga_str);
            }
            bool ok1 = false, ok2 = false, ok3 = false;
            uint32_t t0    = (uint32_t)(fields[1].toDouble(&ok1));
            uint32_t t1    = (uint32_t)(fields[3].toDouble(&ok2));
            uint16_t state = (uint16_t)(fields[5].toDouble(&ok3));
            if(!(ok1&&ok2&&ok3)) {
                good_server_load = false;
                ui->textBrowser->append(QString("Problem getting ") + QString::number(iState) + "th from FPGA server: " + valFpga_str);
                ui->textBrowser->append(QString("t0 = %1").arg(t0));
                ui->textBrowser->append(QString("t1 = %1").arg(t1));
                ui->textBrowser->append(QString("state = %1").arg(state));
                ui->textBrowser->append(fields[0]);
                ui->textBrowser->append(fields[1]);
                ui->textBrowser->append(fields[2]);
                ui->textBrowser->append(fields[3]);
                ui->textBrowser->append(fields[4]);
                ui->textBrowser->append(fields[5]);
            }
        }
    }
    
    if(!good_server_load){
        zmq_send(socket_fpga,"RESET RUNFILE",13,0);
        nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
        return false;
    }else{
        zmq_send(socket_fpga,"LOAD FPGA",9,0);
        nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
        assert(nbytes != -1);
        fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
        QString load_str(fpga_recv_buffer);
        if(load_str!="FPGA LOADED GOOD"){
            zmq_send(socket_fpga,"DONT START FPGA",15,0);
            nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
            return false;
        }else{
            // FPGA is primed and ready
            // send either "START FPGA NOW" or "START FPGA HMGX"
            // but do that somewhere else, once you've checked that everything is ready
            return true;
        }
    }
    return false; // should never get here
#else
    ui->textBrowser->append(QString("TEST: Attempting to load runfile to FPGA."));
    return true;
#endif
}

void ControlGUI::waitForGoThenStart(bool firstcall)
{
    enableChannelButtons(false);
    
    static bool midas_started       = false;
    static bool fastcom_daq_running = false;

    if(firstcall) {
        std::cout << "First waitForGoCall: " << QDateTime::currentMSecsSinceEpoch() << "\n";
        midas_started       = false;
        fastcom_daq_running = false;
    }

    if(!midas_started) {

        // -----------------------------------------------------
        // If we aren't starting midas with the DAQ then set the
        // midas_started to true and set a 50 ms timer to call
        // the function again and then return. If the slow control
        // is controling the DAQ then attempt to start the daq with
        // the start_robs_daq function.  Then start a 1000 ms timer
        // and recall this function. If the DAQ doesn't start it
        // looks like this function will attempt to start the daq every s.

        if(!(ui->startMidasCheckbox->isChecked())) {
            midas_started = true;
            waitForGoTimer->start(50);
            return;
        } else if(ui->startMidasCheckbox->isChecked()){
		midas_started = start_robs_daq();
        }

	waitForGoTimer->start(1000);

	return;
    }

    waitForGoTimer->stop();
    QTimer::singleShot(5000, [this]() {
        ;
    });

    int nbytes;
    if(!(ui->waitForHmGXCheckBox->isChecked())) { // If we aren't keying off the HmGX then just start the cycle
	fpga_run_in_progress = true;
        zmq_send(socket_fpga,"START FPGA NOW",14,0);
        startTheCycle();
        nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
        assert(nbytes != -1);
    }else{
        fpga_run_in_progress = true;
	
	set_nth_bit(fpga_current_state,kBeamEnableCh,!kBeamOn);
        QString cmd = QString("START FPGA HMGX ") + QString::number((int)(fpga_current_state));
	set_nth_bit(fpga_current_state,kBeamEnableCh,kBeamOn);
        zmq_send(socket_fpga, cmd.toLocal8Bit().data(), cmd.size(), 0);
        nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
        assert(nbytes != -1);
        fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
        QString trigger_str1(fpga_recv_buffer);
        
        bool bad = false;
        
        if (trigger_str1=="FIRST POS EDGE HMGX") {
            std::cout << "First pos_edge_HmGX: " << QDateTime::currentMSecsSinceEpoch() << std::endl;
            std::cout << "----------------------------------- " << std::endl;
            zmq_send(socket_fpga,"KEEP GOING",10,0); // meaningless command but I have to send something
            nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
            assert(nbytes != -1);
            fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
            QString trigger_str2(fpga_recv_buffer);
            if (trigger_str2=="SECOND POS EDGE HMGX") {
                std::cout << "Second pos_edge_HmGX: " << QDateTime::currentMSecsSinceEpoch() << std::endl;
                startTheCycle();
                std::cout << "----------------------------------- " << std::endl;
                ui->textBrowser->append("Found two HmGX positive edges, starting run");
            }
            else { // trigger_str2=="HMGX TIMEOUT", or some unknown behavior
                std::cout << "Second pos_edge_HmGX not found, timed out: " << QDateTime::currentMSecsSinceEpoch() << std::endl;
                ui->textBrowser->append("Timed out trying to find the second HmGX positive edge, not starting run");
                bad = true;
            }
        } else { // trigger_str1=="HMGX TIMEOUT", or some unknown behavior
            std::cout << "First pos_edge_HmGX not found, timed out: " << QDateTime::currentMSecsSinceEpoch() << std::endl;
            ui->textBrowser->append("Timed out trying to find the first HmGX positive edge, not starting run");
            bad = true;
        }
        
        if (bad) {
            std::cout << "----------------------------------- " << std::endl;
            waitForGoTimer->stop();
            enableChannelButtons(true);
            ui->startButton->setEnabled(true);
            ui->abortButton->setEnabled(false);
            progressLine->setVisible(false);
        }
    }
}

bool ControlGUI::start_robs_daq()
{
#if !TESTDAQ
    QString comment = ui->commentLineEdit->text();
    std::fstream fcomment;
    fcomment.open("/home/daq/DAQ/daq_dev/.run_comment.txt",std::fstream::out);
    fcomment << comment.toStdString().data();
    fcomment.close();
    //-----------------------------------------------------------------------------

    int nbytes =0;
    bool not_ready = true;
    do{
        // add in a check to see if all daq hardware servers are ready.

	std::cout << "Sending sitrep" << std::endl;
        zmq_send(socket_daq,"sitrep",6,0);

        nbytes = zmq_recv(socket_daq, daq_recv_buffer, daq_recv_buffer_size-1, 0);
        daq_recv_buffer[nbytes] = '\0';

        if(nbytes == -1) {
            printf("Encountered Error! \nErrno:%d\n%s\n", errno, zmq_strerror(errno));
            return false;
        }
        std::cout << "Received : " << daq_recv_buffer << std::endl;
        if(strcmp("ready",daq_recv_buffer)==0)
            not_ready=false;
        else
            QThread::sleep(1);
    }while(not_ready);


    zmq_send(socket_daq, "start", 5, 0);
    nbytes = zmq_recv(socket_daq, daq_recv_buffer, daq_recv_buffer_size-1, 0);
    if(nbytes == -1) {
    	printf("Encountered Error! \nErrno:%d\n%s\n", errno, zmq_strerror(errno));
    	return false;
    }

    daq_recv_buffer[nbytes] = '\0';
    std::cout << "daq_recv_buffer: " << daq_recv_buffer << std::endl;
    char *seg;
    if(strcmp("stop",daq_recv_buffer) !=0){
        seg = strtok(daq_recv_buffer," ");
    }
    char cmd[25];
    char number[25];

    if(seg !=NULL && strcmp("starting",seg) == 0){
	   std::cout << "reading cmd" << std::endl;
           strcpy(cmd,seg);
	   std::cout << "cmd: " << cmd << std::endl;
           seg = strtok(NULL," ");
	   std::cout << "reading cmd" << std::endl;
	   std::cout << "seg: " << seg << std::endl;
           strcpy(number,seg);
	   std::cout << "number: " << number << std::endl;
    }
    run_number = (int)atoi(number);

#if BLIND
    setdb(clean_time,count_time);
#else
    setdb(0,0);
#endif

    std::cout  << run_number << std::endl;

    make_elog_post();

    if(!strcmp("starting", (const char*)daq_recv_buffer)) {
        return true;
    }
#endif
    return false;
}

bool ControlGUI::stop_robs_daq()
{
#if !TESTDAQ
    zmq_send(socket_daq, "stop", 4, 0);
    int nbytes = zmq_recv(socket_daq, daq_recv_buffer, daq_recv_buffer_size-1, 0);
    if(nbytes == -1) {
		std::cout << "DAQ not found!!!"<< std::endl;
		daq_recv_buffer[0] = '\0';
		zmq_close(socket_daq);
		socket_daq = zmq_socket(context,ZMQ_REQ);
		int timeout = 5000;
		zmq_setsockopt(socket_daq,ZMQ_RCVTIMEO,&timeout, sizeof (timeout));
		int rc = zmq_connect(socket_daq, "ipc:///tmp/zeromq_daq");
		assert(rc == 0);
		return false;
	}
    // assert(nbytes != -1);
    daq_recv_buffer[nbytes] = '\0';
    QThread::sleep(15);
    if(!strcmp("stopping", (const char*)daq_recv_buffer)) {
        return true;
    }
#endif
    return false;
}

bool ControlGUI::start_midas()
{
#if !TESTDAQ
    QProcess process(this);
    QStringList args;
    // no longer in use.

    args << "-e" << "Default"
         << "-c" << "ls Runinfo/State";
    process.start("odbedit", args);
    process.waitForFinished();
    QString midas_run_state= process.readAllStandardOutput().simplified();
    ui->textBrowser->append("MIDAS run state:");
    ui->textBrowser->append(midas_run_state);
    int state = -1;
    QStringList fields = midas_run_state.split(" ");
    if(fields.size() == 2) {
        state = fields.last().toInt();
    }

    args.clear();
    args << "-e" << "Default"
         << "-c" << "ls \"Runinfo/Transition in progress\"";
    process.start("odbedit", args);
    process.waitForFinished();
    QString midas_transition_in_progress = process.readAllStandardOutput().simplified();
    ui->textBrowser->append(midas_transition_in_progress);
    fields = midas_transition_in_progress.split(" ");
    int transition_in_progress = -1;
    if(fields.size() == 4) {
        transition_in_progress = fields.last().toInt();
    }

    ui->textBrowser->append("state=" + QString::number(state)
                            + ", transition=" + QString::number(transition_in_progress));
    if((state != 1) || (transition_in_progress != 0)) {
        ui->textBrowser->append("MIDAS appears to be running or transitioning. Waiting 1s before checking again...");
        return false;
    }


    QString comment = ui->commentLineEdit->text();

    args.clear();
    int total_run_duration = int(ui->cyclePeriodLCD->value() + 10.5-deltat);
    args << QString().setNum(total_run_duration)
             << QString("\"" + comment + "\"");
   // qDebug() << args;

    process.start(kControlDir + QString("start_script"), args);
    process.waitForFinished();
    QThread::sleep(2);
    make_elog_post();
#endif
    return true;
}

bool ControlGUI::setdb(double ft,double ct)
{
#if !TESTDAQ
    /*QProcess process(this);
    QStringList args;  // updated for 2016-2017 runs
    args << QString().setNum(ct,'g',10) << QString().setNum(ft,'g',10);
#if DEBUG_BLIND
    printf(" Cleaner : %f Dagger : %f \n",ct,ft);   
#endif
    process.start(kControlDir + QString("db_script"),args);
    process.waitForFinished();*/
    std::fstream fout;
    fout.open("/home/daq/.stuff/.collection_2016.txt",std::fstream::out | std::fstream::app);
    fout << run_number << "\t" <<  std::setprecision(10) << ct;
    fout << "\t" <<  std::setprecision(10) << ft << "\n";
    fout.close();

    QThread::sleep(2);
#if DEBUG_BLIND
    printf(" Cleaner : %f Dagger : %f \n",ct,ft);
#endif
#endif
    return true;
}

void write_bool_event_list(QTextStream &out, QList<ControlEvent<bool> > events)
{
    for(int i=0; i < events.size(); i++) {
        out << " (" << qSetRealNumberPrecision(6) << events.at(i).bltime() << ", ";
        out << (events.at(i).state() ? "true" : "false") << ")";
    }
    out << endl;
}

void ControlGUI::make_elog_post()
{
    QProcess process(this);
    QStringList args;

    QFile fparams(kControlDir + QString("tmp/runparams.txt"));
    if(fparams.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&fparams);
        out << "Run Number : " << this->run_number << endl;
        out << "Comment : " << ui->commentLineEdit->text() << endl;
        out << "Beam Veto Events (time, new state) :";
        write_bool_event_list(out, beamEnableStates);
        out << "Cleaner Down Events (time, new state) :";
        write_bool_event_list(out, cleanerStates);
        out << "West Gate Valve Open Events (time, new state) :";
        write_bool_event_list(out, westGateValveStates);
        out << "Roundhouse Gate Valve Open Events (time, new state) :";
        write_bool_event_list(out, roundhouseGateValveStates);
        out << "Butterfly Valve Closed Events (time, new state) :";
        write_bool_event_list(out, butterflyStates);

        out << "Trapdoor Events (time, new state) :";
        for(int i=0; i < trapdoorStates.size(); i++) {
            out << " (" << qSetRealNumberPrecision(6) << trapdoorStates.at(i).bltime() << ", ";
            out << trapdoorStates.at(i).state() << ")";
        }
        out << endl;

        out << "Dagger Position Events (time, new position in steps) :";
        for(int i=0; i < daggerPositions.size(); i++) {

                out << " (" << qSetRealNumberPrecision(6) << daggerPositions.at(i).bltime() << ", ";
                out << qSetRealNumberPrecision(6) << daggerPositions.at(i).state() << ")";
        }
        out << endl;

        out << "Giant Cleaner Position Events (time, new position in steps) :";
        for(int i=0; i < giantCleanerPositions.size(); i++) {

                out << " (" << qSetRealNumberPrecision(6) << giantCleanerPositions.at(i).bltime() << ", ";
                out << qSetRealNumberPrecision(6) << giantCleanerPositions.at(i).state() << ")";

        }
        out << endl;

        out << "------------------------------------" << endl;
    }
    fparams.close();

    if(!(ui->plot->savePng(kControlDir + QString("tmp/rundiagram.png")))) {
        ui->textBrowser->append("Failed to save rundiagram.png");
    }

    args << kControlDir + QString("tmp/runparams.txt");
    args << kControlDir + QString("tmp/rundiagram.png");

#if !TESTDAQ
    qDebug() << kControlDir + QString("controlpost") << args;
    process.start(kControlDir + QString("controlpost"), args);
    process.waitForFinished();
#endif
}

void ControlGUI::startTheCycle()
{
    cycleTime.start();
    elapsedTimer->start(1000); // update displayed elapsed time every 1 sec
#if FILLCHECK
	fillCheckTimer->start(1000*5);
	while(zmq_recv(socket_fillCheck, fillCheck_recv_buffer, fillCheck_recv_buffer_size-1, ZMQ_DONTWAIT) > -1) ; //Empty out messages from fill check socket
#endif

    doNextBeamTransition(true);
    doNextWestGateValveTransition(true);
    doNextRoundhouseGateValveTransition(true);
    doNextCleanerTransition(true);
    doNextButterflyTransition(true);
    doNextTrapdoorTransition(true);
    doNextDaggerTransition(true);
    doNextGiantCleanerTransition(true);
    doNextnEDMTransistion(true);
    doNextPumpOutPortTransistion(true);
    
    ui->textBrowser->append(QString("Starting the cycle."));

    QThread::sleep(5);
    zmq_send(socket_daq, "plot", 4, 0);
    int nbytes = zmq_recv(socket_daq, daq_recv_buffer, daq_recv_buffer_size-1, 0);
    
#if !TESTDAQ
    if(nbytes <= 0) std::cout << "Unable to start DAQ system" << std::endl;
    else            std::cout << "Able to start DAQ system" << std::endl;
#else
    std::cout << "TEST: Able to start DAQ system" << std::endl;
#endif

    ui->abortButton->setEnabled(true);
}

void ControlGUI::doNextPumpOutPortTransistion(bool firstcall)
{

    if(firstcall) iPumpoutPortTransistion = 0;

    if(iPumpoutPortTransistion < 0) return;

    if(iPumpoutPortTransistion < PumpOutStates.size()){
        //ui->D2PumpOutradioButton->setChecked(PumpOutStates.at(iPumpoutPortTransistion).state());
        iPumpoutPortTransistion++;
        if(iPumpoutPortTransistion < PumpOutStates.size()){
            double next_interval = PumpOutStates.at(iPumpoutPortTransistion).time()
                    - PumpOutStates.at(iPumpoutPortTransistion - 1).time();
            pumpoutportTimer->start(1000.*next_interval);
        } else {
            iPumpoutPortTransistion = -1;
        }
    }
}

void ControlGUI::doNextSourceCtrlTransistion(bool firstcall){
    if(firstcall) iSourceCtrlTransistion = 0;

    if(iSourceCtrlTransistion < 0) return;

    if(iSourceCtrlTransistion < SourceCtrlStates.size()){
        // set the state logic based on the desired output logic of the channel
        bool state = (kSourceCtrlCh == SourceCtrlStates.at(iSourceCtrlTransistion).state());
        //TODO: update when this is actually built into the GUI, may have to change some logic around
        //if(!checked^kSourceCtrlOn) fpga_current_state |=   1<<kSourceCtrlCh;
        //else                       fpga_current_state &= ~(1<<kSourceCtrlCh);
        //ui->D2PumpOutradioButton->setChecked(PumpOutStates.at(iPumpoutPortTransistion).state());
        iSourceCtrlTransistion++;
        if(iSourceCtrlTransistion < SourceCtrlStates.size()){
            double next_interval = SourceCtrlStates.at(iSourceCtrlTransistion).time()
                    - SourceCtrlStates.at(iSourceCtrlTransistion - 1).time();
            sourceCtrlTimer->start(1000.*next_interval);
        } else {
            iSourceCtrlTransistion = -1;
        }
    }
}

void ControlGUI::doNextnEDMTransistion(bool firstcall)
{
    if(firstcall) inEDMTransition = 0;  // set the beam transition counter to 0

    if(inEDMTransition < nEDMstates.size()) {
    #if DEBUG_BLIND
        std::cout << "nEDM GV enable time : " << cycleTime.elapsed() << std::endl;
    #endif
        ui->nEDMGVButton->setChecked(nEDMstates.at(inEDMTransition).state());
        inEDMTransition++;

        // Check if this is not the last transition of the cycle
        if(inEDMTransition < nEDMstates.size()) {
            double next_interval = nEDMstates.at(inEDMTransition).time()
                    - nEDMstates.at(inEDMTransition - 1).time();
            nEDMGV_Timer->start(1000*next_interval);
        } else {
            inEDMTransition = -1;
        }
    } else {
        inEDMTransition = -1;
    }
}

// FIXME: should use templates or something for all of these...
void ControlGUI::doNextBeamTransition(bool firstcall)
{
    if(firstcall) iBeamTransition = 0;  // set the beam transition counter to 0

    if(iBeamTransition < 0) return;

    if(iBeamTransition < beamEnableStates.size()) {
    #if DEBUG_BLIND
        std::cout << "Beam enable time : " << cycleTime.elapsed() << std::endl;
    #endif
        ui->beamEnableRadio->setChecked(beamEnableStates.at(iBeamTransition).state());
        iBeamTransition++;

        // Check if this is not the last transition of the cycle
        if(iBeamTransition < beamEnableStates.size()) {
            double next_interval = beamEnableStates.at(iBeamTransition).time() - beamEnableStates.at(iBeamTransition - 1).time();
            beamTransitionTimer->start(1000*next_interval);
        } else {
            iBeamTransition = -1;
        }
    } else {
        iBeamTransition = -1;
    }
}

void ControlGUI::doNextWestGateValveTransition(bool firstcall)
{
    if(firstcall) iWestGateValveTransition = 0;

    if(iWestGateValveTransition < 0) return;

    if(iWestGateValveTransition < westGateValveStates.size()) {
    #if DEBUG_BLIND
        std::cout << "West GV movement time : " << cycleTime.elapsed() << std::endl;
    #endif
        ui->westGateValveRadio->setChecked(westGateValveStates.at(iWestGateValveTransition).state());
        iWestGateValveTransition++;

        // Check if this is not the last transition of the cycle
        if(iWestGateValveTransition < westGateValveStates.size()) {
            double next_interval = westGateValveStates.at(iWestGateValveTransition).time() - westGateValveStates.at(iWestGateValveTransition - 1).time();
            westGateValveTransitionTimer->start(1000*next_interval);
        } else {
            iWestGateValveTransition = -1;
        }
    } else {
        iWestGateValveTransition = -1;
    }
}

void ControlGUI::doNextRoundhouseGateValveTransition(bool firstcall)
{
    if(firstcall) iRoundhouseGateValveTransition = 0;

    if(iRoundhouseGateValveTransition < 0) return;

    if(iRoundhouseGateValveTransition < roundhouseGateValveStates.size()) {
    #if DEBUG_BLIND
        std::cout << "West GV movement time : " << cycleTime.elapsed() << std::endl;
    #endif
        ui->roundhouseGateValveRadio->setChecked(roundhouseGateValveStates.at(iRoundhouseGateValveTransition).state());
        iRoundhouseGateValveTransition++;

        // Check if this is not the last transition of the cycle
        if(iRoundhouseGateValveTransition < roundhouseGateValveStates.size()) {
            double next_interval = roundhouseGateValveStates.at(iRoundhouseGateValveTransition).time() - roundhouseGateValveStates.at(iRoundhouseGateValveTransition - 1).time();
            roundhouseGateValveTransitionTimer->start(1000*next_interval);
        } else {
            iRoundhouseGateValveTransition = -1;
        }
    } else {
        iRoundhouseGateValveTransition = -1;
    }
}

void ControlGUI::doNextCleanerTransition(bool firstcall)
{
    if(firstcall) iCleanerTransition = 0;

    if(iCleanerTransition < 0) return;

    if(iCleanerTransition < cleanerStates.size()) { //swapped out this line for the following during debugging 12/10/18
        ui->cleanerRadio->setChecked(cleanerStates.at(iCleanerTransition).state());
        iCleanerTransition++;

        // Check if this is not the last transition of the cycle
        if(iCleanerTransition < cleanerStates.size()) {
            double next_interval = cleanerStates.at(iCleanerTransition).time() - cleanerStates.at(iCleanerTransition - 1).time();
            cleanerTransitionTimer->start(1000*next_interval);
        } else {
            iCleanerTransition = -1;
        }
    } else {
        iCleanerTransition = -1;
    }
}

void ControlGUI::doNextButterflyTransition(bool firstcall)
{
    if(firstcall) iButterflyTransition = 0;

    if(iButterflyTransition < 0) return;

    if(iButterflyTransition < butterflyStates.size()) {
        ui->butterflyRadio->setChecked(butterflyStates.at(iButterflyTransition).state());
        iButterflyTransition++;

        // Check if this is not the last transition of the cycle
        if(iButterflyTransition < butterflyStates.size()) {
            double next_interval = butterflyStates.at(iButterflyTransition).time() - butterflyStates.at(iButterflyTransition - 1).time();
            butterflyTransitionTimer->start(1000*next_interval);
        } else {
            iButterflyTransition = -1;
        }
    } else {
        iButterflyTransition = -1;
    }
}

void ControlGUI::doNextTrapdoorTransition(bool firstcall)
{
    if(firstcall) iTrapdoorTransition = 0;

    if(iTrapdoorTransition < 0) return;

    if(iTrapdoorTransition < trapdoorStates.size()) {
    #if DEBUG_BLIND
        std::cout << "Trap door movement time : " << cycleTime.elapsed() << std::endl;
    #endif
        int i_item = ui->trapdoorComboBox->findText(trapdoorStates.at(iTrapdoorTransition).state());
        int prev_i_item = ui->trapdoorComboBox->currentIndex();
        ui->trapdoorComboBox->setCurrentIndex(i_item);
        if(i_item == prev_i_item) { // currentIndexChanged signal is not emitted in this case
            trapdoor_index_changed(i_item); // call the slot directly in this case
        }
        iTrapdoorTransition++;

        // Check if this is not the last transition of the cycle
        if(iTrapdoorTransition < trapdoorStates.size()) {
            double next_interval = trapdoorStates.at(iTrapdoorTransition).time() -
                trapdoorStates.at(iTrapdoorTransition - 1).time();
            trapdoorTransitionTimer->start(1000*next_interval);
        } else {
            iTrapdoorTransition = -1;
        }
    } else {
        iTrapdoorTransition = -1;
    }
}

void ControlGUI::doNextDaggerTransition(bool firstcall)
{
#if TESTDAGGER
   ui->textBrowser->append(QString( "TEST: Moving the Dagger"));
#endif
    if(firstcall) iDaggerTransition = 0;

    if(iDaggerTransition < 0) return;

    if(iDaggerTransition < daggerPositions.size()) {
    #if DEBUG_BLIND
        std::cout << "Dagger movement time : " << cycleTime.elapsed() << std::endl;
    #endif
        ui->daggerSpinBox->setValue(daggerPositions.at(iDaggerTransition).state());
        dagger_button_pushed();
        iDaggerTransition++;

        // Check if this is not the last transition of the cycle
        if(iDaggerTransition < daggerPositions.size()) {
            double next_interval = daggerPositions.at(iDaggerTransition).time() - daggerPositions.at(iDaggerTransition - 1).time();
        #if DEBUG_BLIND
            std::cout << "Next dagger transition in : " << next_interval*1000 << std::endl;
        #endif
            daggerTransitionTimer->start((int)(1000.*next_interval));
        } else {
            iDaggerTransition = -1;
        }
    } else {
        iDaggerTransition = -1;
    }
}

void ControlGUI::doNextGiantCleanerTransition(bool firstcall)
{
#if TESTGIANTCLEANER
    ui->textBrowser->append(QString("TEST: Moving the GiantCleaner"));
#endif
    if(firstcall) iGiantCleanerTransition = 0;

    if(iGiantCleanerTransition < 0) return;

    if(iGiantCleanerTransition < giantCleanerPositions.size()) {
    #if GCFULLACTUATORINUSE
        ui->giantCleanerSpinBox->setValue(giantCleanerPositions.at(iGiantCleanerTransition).state());
    #else
        ui->giantCleanerRadio->setChecked(giantCleanerPositions.at(iGiantCleanerTransition).state()<100001);
    #endif
    
        iGiantCleanerTransition++;

        // Check if this is not the last transition of the cycle
        if(iGiantCleanerTransition < giantCleanerPositions.size()) {
            double next_interval = giantCleanerPositions.at(iGiantCleanerTransition).time() - giantCleanerPositions.at(iGiantCleanerTransition - 1).time();
            giantCleanerTransitionTimer->start(1000*next_interval);
        } else {
            iGiantCleanerTransition = -1;
        }
    } else {
        iGiantCleanerTransition = -1;
    }
}

void ControlGUI::updateElapsedTime()
{
    double elapsed = double(cycleTime.elapsed())/1000;
    
    if(elapsed > clean_time && elapsed < count_time){
       elapsed = clean_time + (elapsed-clean_time)/(2.0-factor);
    } else if(elapsed >= count_time){
       elapsed = clean_time + (count_time-clean_time)/(2.0-factor) + (elapsed-count_time);
    }
    
    if (elapsed<last_check_time) {
        // check if FPGA is in sync
        int nbytes;
        zmq_send(socket_fpga, "CHECK", 5, 0);
        nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1,0);
        assert(nbytes != -1);
        fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
        QString fpga_str(fpga_recv_buffer);
        if(fpga_str=="GOOD SYNC") sync_count=0;
        else {
            sync_count++;
            if (sync_count>=5) {
                ui->textBrowser->append("ERROR!!! FPGA and Slow Control are misaligned!");
            }
        }
    }
    
    ui->elapsedTimeLCD->display(int(elapsed + 0.5)); // rounded to nearest second
    progressLine->start->setCoords(elapsed, -1.);
    progressLine->end->setCoords(elapsed, 7.);
    ui->plot->replot();

    // check if all channels are done
    bool all_done = (iBeamTransition < 0)
                 && (iWestGateValveTransition < 0)
                 && (iRoundhouseGateValveTransition < 0)
                 && (iTrapdoorTransition < 0)
                 && (iCleanerTransition < 0)
                 && (iButterflyTransition < 0)
                 && (iDaggerTransition < 0)
                 && (iGiantCleanerTransition < 0);
    if(all_done) {
        stopTheCycle(false,false);
        QThread::sleep(1);
    }
}

void ControlGUI::abortButton_clicked()
{
    ui->textBrowser->append("Aborting the run sequence");
    fpga_current_state = fpga_initial_state;
    stopTheCycle(true, false);
    QThread::sleep(1);
    enableChannelButtons(true);
    ui->startButton->setEnabled(true);
}

void ControlGUI::stopTheCycle(bool abort_run, bool bad_fill)
{
	int nbytes;

#if GCFULLACTUATORINUSE && !TESTGIANTCLEANER
    ui->textBrowser->append("Trying to clear Giant Cleaner queue.");
    zmq_send(socket_giantCleaner, "ABRT", 4, 0);
    nbytes = zmq_recv(socket_giantCleaner, giantCleaner_recv_buffer, giantCleaner_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    giantCleaner_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString giantCleaner_str(giantCleaner_recv_buffer);
    if(giantCleaner_str=="ABORTED") ui->textBrowser->append("Giant Cleaner queue emptied.");
    else                            ui->textBrowser->append("WARNING! Giant Cleaner could not empty its queue.");
#else
    ui->textBrowser->append("TEST: Giant Cleaner queue emptied.");
#endif
    
#if !TESTDAGGER
    ui->textBrowser->append("Trying to clear Dagger queue.");
    zmq_send(socket_dagger, "ABRT", 4, 0);
    nbytes = zmq_recv(socket_dagger, dagger_recv_buffer, dagger_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    dagger_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString dagger_str(dagger_recv_buffer);
    if(dagger_str=="ABORTED") ui->textBrowser->append("Dagger queue emptied.");
    else                      ui->textBrowser->append("WARNING! Dagger could not empty its queue.");
#else
    ui->textBrowser->append("TEST: Dagger queue emptied.");
#endif
    
#if !TESTFPGA
    if(abort_run){
        ui->textBrowser->append("Trying to abort FPGA.");
        zmq_send(socket_fpga,"ABORT",5,0);
        nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
        assert(nbytes != -1);
        fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
        QString fpga_str(fpga_recv_buffer);
        if(fpga_str=="SENT ABORT") ui->textBrowser->append("FPGA server sent abort message to FPGA.");
        else                       ui->textBrowser->append("WARNING! FPGA server failed to send abort message to FPGA.");
    }
    
    ui->abortButton->setEnabled(false);

    if(ui->startMidasCheckbox->isChecked()) {
        //midas_started = start_midas();
        bool stopped = stop_robs_daq();
    }
    
    zmq_send(socket_fpga, "SLOW CONTROL AT END OF RUN", 26, 0);
    nbytes = zmq_recv(socket_fpga, fpga_recv_buffer, fpga_recv_buffer_size-1, 0);
    assert(nbytes != -1);
    fpga_recv_buffer[nbytes] = '\0'; // properly NULL terminate received string
    QString fpga_str(fpga_recv_buffer);
    if(fpga_str=="RUN ABORTED") {
        ui->textBrowser->append("FPGA reports that the run was aborted.");
    }
    else if(fpga_str=="RUN GOOD") ui->textBrowser->append("FPGA reports that the run is over.");
    else                          ui->textBrowser->append("FPGA reports that it had trouble doing the run.");
#else
    if(abort_run) {
        ui->textBrowser->append("TEST: Trying to abort FPGA.");
        ui->textBrowser->append("TEST: FPGA server sent abort message to FPGA.");
        ui->textBrowser->append("TEST: FPGA reports that the run was aborted.");
    } else {
        ui->textBrowser->append("TEST: FPGA reports that the run is over.");
    }   
#endif

    fpga_run_in_progress = false;

    // Tell the user we are stopping the cycle
    ui->textBrowser->append(QString("Stopping the cycle."));

    // Stopping Timers
    //---------------------------------------------------
    for(int i = 0 ; i < TimerList.size() ; i++) TimerList[i]->stop();

    ui->elapsedTimeLCD->display(0);
    progressLine->setVisible(false);

    ui->plot->replot(); // refresh the plot to hide the progress line

    if(!abort_run && !ui->stopAfterThisRunCheckBox->isChecked() && (iStart < runFileNames.size() - 1)) {
        iStart++;
        // FIXME: add a delay before starting the next cycle?
        startButton_clicked(false, true);
        override_list=false;
        enableChannelButtons(false);
        ui->startButton->setEnabled(false);
    } else {
        enableChannelButtons(true);
        ui->startButton->setEnabled(true);
    }
	
	if(bad_fill) {
		updateTimingPattern("location_of_empty_trap_run");
		startButton_clicked(false, false);
		override_list=false;
	}
}

void ControlGUI::get_midas_odb()
{
    std::fstream fin;
    fin.open("/home/daq/.stuff/.daq_parameters_2017.txt",std::fstream::in);
    double thing1;
    fin >> thing1;
    
#if BLIND
    factor = thing1;
    factor = 1.9; // TODO
#else
    factor = 1.0;
#endif

    fin.close();
}

void ControlGUI::on_loadfileNumber_button_clicked()
{
    int nlist = ui->listnumber_spinbox->value();
    iStart  = nlist;
    updateTimingPattern(runFileNames[iStart]);
    override_list = true;
}

void ControlGUI::fillCheck() {
#if FILLCHECK
	if(ui->fillCheckCheckbox->isChecked()) {
		if(zmq_recv(socket_fillCheck, fillCheck_recv_buffer, fillCheck_recv_buffer_size-1, ZMQ_DONTWAIT) > -1) {
			if(strcmp("bad",fillCheck_recv_buffer) == 0) {
				stopTheCycle(false, true);
				QThread::sleep(1);
				enableChannelButtons(true);
                ui->startButton->setEnabled(true);
			}
		}
	}
#else
    return;
#endif
}


QTimer* ControlGUI::SetMyTimer(bool isSingleShot ,bool isPrecise,
                            const char* typeofConnection)
{
    QTimer* MyTimer = new QTimer(this);

    MyTimer->setSingleShot(isSingleShot);

    if(isPrecise)
        MyTimer->setTimerType(Qt::PreciseTimer);
    std::cout << "connecting to : " << typeofConnection << std::endl;

    connect(MyTimer, SIGNAL(timeout()),this, typeofConnection);
    TimerList.append(MyTimer);
    return MyTimer;
}

void ControlGUI::InitTimers()
{
    //---------------------------------------------------------------------------
    // Use the SetMyTimer method to instanstiate the QTimers;
    waitForGoTimer                     = SetMyTimer(true,false,SLOT(waitForGoThenStart()));
    beamTransitionTimer                = SetMyTimer(true,true,SLOT(doNextBeamTransition()));
    westGateValveTransitionTimer       = SetMyTimer(true,true,SLOT(doNextWestGateValveTransition()));
    roundhouseGateValveTransitionTimer = SetMyTimer(true,true,SLOT(doNextRoundhouseGateValveTransition()));
    cleanerTransitionTimer             = SetMyTimer(true,true,SLOT(doNextCleanerTransition()));
    trapdoorTransitionTimer            = SetMyTimer(true,true,SLOT(doNextTrapdoorTransition()));
    daggerTransitionTimer              = SetMyTimer(true,true,SLOT(doNextDaggerTransition()));
    giantCleanerTransitionTimer        = SetMyTimer(true,true,SLOT(doNextGiantCleanerTransition()));
    butterflyTransitionTimer           = SetMyTimer(true,true,SLOT(doNextButterflyTransition()));
    nEDMGV_Timer                       = SetMyTimer(true,false,SLOT(doNextnEDMTransistion()));
    elapsedTimer                       = SetMyTimer(false,false,SLOT(updateElapsedTime()));
#if FILLCHECK
    fillCheckTimer                     = SetMyTimer(false,false,SLOT(fillCheck()));
#endif
    pumpoutportTimer                   = SetMyTimer(true,false,SLOT(doNextPumpOutPortTransistion()));
    sourceCtrlTimer                    = SetMyTimer(true,false,SLOT(doNextSourceCtrlTransistion()));
}

void ControlGUI::ConnectSlots()
{
    //----------------------------------------------------------------------------
    // Explicitly connect all the signals to slots.
    connect(ui->beamEnableRadio, SIGNAL(toggled(bool)),this, SLOT(beamEnable_toggled(bool)));
    connect(ui->westGateValveRadio, SIGNAL(toggled(bool)),this, SLOT(westGateValve_toggled(bool)));
    connect(ui->roundhouseGateValveRadio, SIGNAL(toggled(bool)),this, SLOT(roundhouseGateValve_toggled(bool)));
    connect(ui->trapdoorComboBox, SIGNAL(currentIndexChanged(int)),this, SLOT(trapdoor_index_changed(int)));
    connect(ui->cleanerRadio, SIGNAL(toggled(bool)),this, SLOT(cleaner_toggled(bool)));
    connect(ui->daggerPushButton, SIGNAL(clicked()),this, SLOT(dagger_button_pushed()));
    connect(ui->giantCleanerPushButton, SIGNAL(clicked()),this, SLOT(giantCleaner_button_pushed()));
    connect(ui->butterflyRadio, SIGNAL(toggled(bool)),this, SLOT(butterfly_toggled(bool)));
    connect(ui->openFileButton, SIGNAL(clicked()),this, SLOT(openFile_clicked()));
    connect(ui->startButton, SIGNAL(clicked()),this, SLOT(startButton_clicked()));
    connect(ui->abortButton, SIGNAL(clicked()),this, SLOT(abortButton_clicked()));
    connect(ui->nEDMGVButton, SIGNAL(toggled(bool)),this,SLOT(nedmEnable_toggled(bool)));
    connect(ui->giantCleanerRadio, SIGNAL(toggled(bool)),this, SLOT(giantCleaner_toggled(bool)));
}
