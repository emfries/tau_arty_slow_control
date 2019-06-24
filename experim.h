/********************************************************************\

  Name:         experim.h
  Created by:   ODBedit program

  Contents:     This file contains C structures for the "Experiment"
                tree in the ODB and the "/Analyzer/Parameters" tree.

                Additionally, it contains the "Settings" subtree for
                all items listed under "/Equipment" as well as their
                event definition.

                It can be used by the frontend and analyzer to work
                with these information.

                All C structures are accompanied with a string represen-
                tation which can be used in the db_create_record function
                to setup an ODB structure which matches the C structure.

  Created on:   Tue Feb 16 20:20:46 2016

\********************************************************************/

#define EXP_PARAM_DEFINED

typedef struct {
  char      comment[103];
  BOOL      counting_while_implanting_;
  BOOL      run_mode;
  char      which_particle_[32];
  float     counting_time;
  float     tape_motion_time;
  float     implanting_time;
  float     h2_target_pressure;
  float     wedge_thickness;
  float     degrader_thickness;
  float     separator_rigidity;
  INT       tape_moving;
  INT       tape_reel_number;
  BOOL      make_analyzed_root_tree;
  INT       broken_transfer_reads;
  float     storage_time;
  double    cleaner_up;
  double    dagger_down;
  char      commentl[256];
} EXP_PARAM;

#define EXP_PARAM_STR(_name) const char *_name[] = {\
"[.]",\
"Comment = STRING : [103] test of rollover correction in online analyzer dumb dumb dubm dubmdudmb",\
"Counting while Implanting? = BOOL : n",\
"Run Mode = BOOL : n",\
"Which Particle? = STRING : [32] background",\
"Counting Time = FLOAT : 0",\
"Tape motion Time = FLOAT : 0",\
"Implanting Time = FLOAT : 0",\
"H2 Target Pressure = FLOAT : 0",\
"Wedge Thickness = FLOAT : 0",\
"Degrader Thickness = FLOAT : 0",\
"Separator Rigidity = FLOAT : 0",\
"Tape moving = INT : 0",\
"Tape Reel Number = INT : 0",\
"Make Analyzed ROOT Tree = BOOL : n",\
"Broken Transfer Reads = INT : 0",\
"Storage time = FLOAT : 1000",\
"cleaner_up = DOUBLE : 370",\
"dagger_down = DOUBLE : 0",\
"commentl = STRING : [256] testing",\
"",\
NULL }

#define EXP_EDIT_DEFINED

typedef struct {
  char      comment[103];
  float     storage_time;
  BOOL      acquisition_control;
} EXP_EDIT;

#define EXP_EDIT_STR(_name) const char *_name[] = {\
"[.]",\
"Comment = LINK : [35] /Experiment/Run Parameters/Comment",\
"Storage time = LINK : [40] /Experiment/Run Parameters/Storage time",\
"Acquisition_Control = LINK : [37] /Equipment/Trigger/Settings/acq_ctrl",\
"",\
NULL }

#ifndef EXCL_AWESOME

#define AWESOME_PARAM_DEFINED

typedef struct {
  INT       placeholder;
} AWESOME_PARAM;

#define AWESOME_PARAM_STR(_name) const char *_name[] = {\
"[.]",\
"placeholder = INT : 0",\
"",\
NULL }

#endif

#ifndef EXCL_ADC_CALIBRATION

#define ADC_CALIBRATION_PARAM_DEFINED

typedef struct {
  INT       pedestal[8];
  float     software_gain[8];
  float     fadc_voltage_conversion[8];
  double    histo_threshold;
  float     peak_height_calibration[8];
} ADC_CALIBRATION_PARAM;

#define ADC_CALIBRATION_PARAM_STR(_name) const char *_name[] = {\
"[.]",\
"Pedestal = INT[8] :",\
"[0] 30",\
"[1] 30",\
"[2] 30",\
"[3] 30",\
"[4] 30",\
"[5] 30",\
"[6] 30",\
"[7] 30",\
"Software Gain = FLOAT[8] :",\
"[0] 0.1",\
"[1] 0.1",\
"[2] 0.1",\
"[3] 0.1",\
"[4] 0.1",\
"[5] 0.1",\
"[6] 1",\
"[7] 1",\
"FADC Voltage Conversion = FLOAT[8] :",\
"[0] 1",\
"[1] 1",\
"[2] 1",\
"[3] 1",\
"[4] 1",\
"[5] 1",\
"[6] 1",\
"[7] 1",\
"Histo threshold = DOUBLE : 20",\
"Peak Height Calibration = FLOAT[8] :",\
"[0] 1",\
"[1] 1",\
"[2] 1",\
"[3] 1",\
"[4] 1",\
"[5] 1",\
"[6] 1",\
"[7] 1",\
"",\
NULL }

#endif

#ifndef EXCL_GLOBAL

#define GLOBAL_PARAM_DEFINED

typedef struct {
  float     adc_threshold;
} GLOBAL_PARAM;

#define GLOBAL_PARAM_STR(_name) const char *_name[] = {\
"[.]",\
"ADC Threshold = FLOAT : 5",\
"",\
NULL }

#endif

#ifndef EXCL_TRIGGER

#define TRIGGER_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} TRIGGER_COMMON;

#define TRIGGER_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 65535",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 2",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 121",\
"Period = INT : 100",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] ucntau-daq",\
"Frontend name = STRING : [32] Frontend",\
"Frontend file name = STRING : [256] CAENfrontend_v1730.cpp",\
"Status = STRING : [256] Frontend@ucntau-daq",\
"Status color = STRING : [32] #00FF00",\
"Hidden = BOOL : n",\
"",\
NULL }

#define TRIGGER_SETTINGS_DEFINED

typedef struct {
  BOOL      external_triggering;
  BOOL      software_triggering;
  BOOL      accept_overlapping_triggers;
  BOOL      run_pulser;
  INT       pulser_rate;
  INT       pulser_width;
  INT       pulser_base_unit;
  INT       samples_above_threshold;
  INT       post_trigger_samples;
  INT       sampling_time__5us__10us___;
  INT       logic_pol;
  INT       local_concidence_level;
  BYTE      io506;
  INT       zero_suppression_mode;
  BOOL      delay_of_odd_channels;
  INT       sampling_time;
  struct {
    INT       threshold[16];
    INT       width[2];
    INT       majority;
    INT       testpulse;
    INT       enabled_channels;
  } discriminator;
  BOOL      zero_suppression_weight[16];
  BOOL      falling_edge_triggering[16];
  INT       zero_suppression_samples[16];
  INT       zero_suppression_threshold[16];
  INT       dc_offset[16];
  INT       leading_edge_threshold[16];
  BOOL      triggering_mode_mask[16];
  BOOL      active_channels[16];
  struct {
    INT       a[16];
    INT       thr[16];
    INT       k[16];
    INT       m[16];
    INT       ftd[16];
    INT       b[16];
    INT       decimation[16];
    INT       dgain[16];
    INT       trgho[16];
    INT       nsbl[16];
    INT       nspk[16];
    INT       pkho[16];
    INT       blho[16];
    float     enf[16];
    INT       otrej[16];
    INT       trgwin[16];
    INT       twwdt[16];
    INT       mm[16];
  } pha_params;
  BOOL      acq_ctrl;
} TRIGGER_SETTINGS;

#define TRIGGER_SETTINGS_STR(_name) const char *_name[] = {\
"[.]",\
"External Triggering = BOOL : n",\
"Software Triggering = BOOL : n",\
"Accept Overlapping Triggers = BOOL : n",\
"Run Pulser = BOOL : n",\
"Pulser Rate = INT : 1",\
"Pulser Width = INT : 1",\
"Pulser Base Unit = INT : 3",\
"Samples Above Threshold = INT : 5",\
"Post Trigger Samples = INT : 70",\
"Sampling Time (5us, 10us..) = INT : 5",\
"logic_pol = INT : 1",\
"Local concidence Level = INT : 0",\
"IO506 = BYTE : 7",\
"Zero Suppression Mode = INT : 3",\
"Delay of Odd Channels = BOOL : n",\
"Sampling Time = INT : 512",\
"",\
"[Discriminator]",\
"threshold = INT[16] :",\
"[0] 50",\
"[1] 30",\
"[2] 100",\
"[3] 100",\
"[4] 100",\
"[5] 100",\
"[6] 100",\
"[7] 100",\
"[8] 100",\
"[9] 100",\
"[10] 100",\
"[11] 100",\
"[12] 200",\
"[13] 200",\
"[14] 200",\
"[15] 200",\
"width = INT[2] :",\
"[0] 5",\
"[1] 5",\
"majority = INT : 1",\
"testpulse = INT : 255",\
"Enabled_Channels = INT : 65535",\
"",\
"[.]",\
"Zero Suppression Weight = BOOL[16] :",\
"[0] n",\
"[1] n",\
"[2] n",\
"[3] n",\
"[4] n",\
"[5] n",\
"[6] n",\
"[7] n",\
"[8] n",\
"[9] n",\
"[10] n",\
"[11] n",\
"[12] n",\
"[13] n",\
"[14] n",\
"[15] n",\
"Falling Edge Triggering = BOOL[16] :",\
"[0] y",\
"[1] y",\
"[2] y",\
"[3] y",\
"[4] y",\
"[5] y",\
"[6] y",\
"[7] y",\
"[8] y",\
"[9] y",\
"[10] y",\
"[11] y",\
"[12] y",\
"[13] y",\
"[14] y",\
"[15] y",\
"Zero Suppression Samples = INT[16] :",\
"[0] 2",\
"[1] 2",\
"[2] 2",\
"[3] 2",\
"[4] 2",\
"[5] 2",\
"[6] 2",\
"[7] 2",\
"[8] 2",\
"[9] 2",\
"[10] 2",\
"[11] 2",\
"[12] 2",\
"[13] 2",\
"[14] 2",\
"[15] 2",\
"Zero Suppression Threshold = INT[16] :",\
"[0] 12600",\
"[1] 14600",\
"[2] 14600",\
"[3] 14600",\
"[4] 14600",\
"[5] 14900",\
"[6] 14900",\
"[7] 14900",\
"[8] 14900",\
"[9] 14900",\
"[10] 14900",\
"[11] 14900",\
"[12] 14900",\
"[13] 14900",\
"[14] 15010",\
"[15] 15010",\
"DC Offset = INT[16] :",\
"[0] 9830",\
"[1] 6553",\
"[2] 6553",\
"[3] 6553",\
"[4] 6356",\
"[5] 6553",\
"[6] 6553",\
"[7] 6094",\
"[8] 6586",\
"[9] 6291",\
"[10] 6553",\
"[11] 6225",\
"[12] 5898",\
"[13] 6094",\
"[14] 6553",\
"[15] 5898",\
"Leading Edge Threshold = INT[16] :",\
"[0] 12500",\
"[1] 14500",\
"[2] 14500",\
"[3] 14500",\
"[4] 14500",\
"[5] 14500",\
"[6] 14500",\
"[7] 14500",\
"[8] 14500",\
"[9] 14500",\
"[10] 14500",\
"[11] 14500",\
"[12] 14500",\
"[13] 14500",\
"[14] 15000",\
"[15] 15000",\
"Triggering Mode Mask = BOOL[16] :",\
"[0] y",\
"[1] n",\
"[2] n",\
"[3] n",\
"[4] y",\
"[5] y",\
"[6] y",\
"[7] y",\
"[8] y",\
"[9] y",\
"[10] y",\
"[11] y",\
"[12] y",\
"[13] y",\
"[14] y",\
"[15] y",\
"Active Channels = BOOL[16] :",\
"[0] n",\
"[1] n",\
"[2] n",\
"[3] n",\
"[4] y",\
"[5] y",\
"[6] y",\
"[7] y",\
"[8] n",\
"[9] n",\
"[10] n",\
"[11] n",\
"[12] n",\
"[13] n",\
"[14] n",\
"[15] n",\
"",\
"[PHA_params]",\
"a = INT[16] :",\
"[0] 8",\
"[1] 8",\
"[2] 8",\
"[3] 8",\
"[4] 8",\
"[5] 8",\
"[6] 8",\
"[7] 8",\
"[8] 8",\
"[9] 8",\
"[10] 8",\
"[11] 8",\
"[12] 8",\
"[13] 8",\
"[14] 8",\
"[15] 8",\
"thr = INT[16] :",\
"[0] 15",\
"[1] 15",\
"[2] 15",\
"[3] 15",\
"[4] 15",\
"[5] 15",\
"[6] 15",\
"[7] 15",\
"[8] 15",\
"[9] 15",\
"[10] 15",\
"[11] 15",\
"[12] 15",\
"[13] 15",\
"[14] 15",\
"[15] 15",\
"k = INT[16] :",\
"[0] 100",\
"[1] 100",\
"[2] 100",\
"[3] 100",\
"[4] 100",\
"[5] 100",\
"[6] 100",\
"[7] 100",\
"[8] 100",\
"[9] 100",\
"[10] 100",\
"[11] 100",\
"[12] 100",\
"[13] 100",\
"[14] 100",\
"[15] 100",\
"m = INT[16] :",\
"[0] 100",\
"[1] 100",\
"[2] 100",\
"[3] 100",\
"[4] 100",\
"[5] 100",\
"[6] 100",\
"[7] 100",\
"[8] 100",\
"[9] 100",\
"[10] 100",\
"[11] 100",\
"[12] 100",\
"[13] 100",\
"[14] 100",\
"[15] 100",\
"ftd = INT[16] :",\
"[0] 50",\
"[1] 50",\
"[2] 50",\
"[3] 50",\
"[4] 50",\
"[5] 50",\
"[6] 50",\
"[7] 50",\
"[8] 50",\
"[9] 50",\
"[10] 50",\
"[11] 50",\
"[12] 50",\
"[13] 50",\
"[14] 50",\
"[15] 50",\
"b = INT[16] :",\
"[0] 100",\
"[1] 100",\
"[2] 100",\
"[3] 100",\
"[4] 100",\
"[5] 100",\
"[6] 100",\
"[7] 100",\
"[8] 100",\
"[9] 100",\
"[10] 100",\
"[11] 100",\
"[12] 100",\
"[13] 100",\
"[14] 100",\
"[15] 100",\
"decimation = INT[16] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"dgain = INT[16] :",\
"[0] 1",\
"[1] 1",\
"[2] 1",\
"[3] 1",\
"[4] 1",\
"[5] 1",\
"[6] 1",\
"[7] 1",\
"[8] 1",\
"[9] 1",\
"[10] 1",\
"[11] 1",\
"[12] 1",\
"[13] 1",\
"[14] 1",\
"[15] 1",\
"trgho = INT[16] :",\
"[0] 100",\
"[1] 100",\
"[2] 100",\
"[3] 100",\
"[4] 100",\
"[5] 100",\
"[6] 100",\
"[7] 100",\
"[8] 100",\
"[9] 100",\
"[10] 100",\
"[11] 100",\
"[12] 100",\
"[13] 100",\
"[14] 100",\
"[15] 100",\
"nsbl = INT[16] :",\
"[0] 4",\
"[1] 4",\
"[2] 4",\
"[3] 4",\
"[4] 4",\
"[5] 4",\
"[6] 4",\
"[7] 4",\
"[8] 4",\
"[9] 4",\
"[10] 4",\
"[11] 4",\
"[12] 4",\
"[13] 4",\
"[14] 4",\
"[15] 4",\
"nspk = INT[16] :",\
"[0] 16",\
"[1] 16",\
"[2] 16",\
"[3] 16",\
"[4] 16",\
"[5] 16",\
"[6] 16",\
"[7] 16",\
"[8] 16",\
"[9] 16",\
"[10] 16",\
"[11] 16",\
"[12] 16",\
"[13] 16",\
"[14] 16",\
"[15] 16",\
"pkho = INT[16] :",\
"[0] 10",\
"[1] 10",\
"[2] 10",\
"[3] 10",\
"[4] 10",\
"[5] 10",\
"[6] 10",\
"[7] 10",\
"[8] 10",\
"[9] 10",\
"[10] 10",\
"[11] 10",\
"[12] 10",\
"[13] 10",\
"[14] 10",\
"[15] 10",\
"blho = INT[16] :",\
"[0] 50",\
"[1] 50",\
"[2] 50",\
"[3] 50",\
"[4] 50",\
"[5] 50",\
"[6] 50",\
"[7] 50",\
"[8] 50",\
"[9] 50",\
"[10] 50",\
"[11] 50",\
"[12] 50",\
"[13] 50",\
"[14] 50",\
"[15] 50",\
"enf = FLOAT[16] :",\
"[0] 1",\
"[1] 1",\
"[2] 1",\
"[3] 1",\
"[4] 1",\
"[5] 1",\
"[6] 1",\
"[7] 1",\
"[8] 1",\
"[9] 1",\
"[10] 1",\
"[11] 1",\
"[12] 1",\
"[13] 1",\
"[14] 1",\
"[15] 1",\
"otrej = INT[16] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"trgwin = INT[16] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"twwdt = INT[16] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"MM = INT[16] :",\
"[0] 25000",\
"[1] 25000",\
"[2] 25000",\
"[3] 25000",\
"[4] 25000",\
"[5] 25000",\
"[6] 25000",\
"[7] 25000",\
"[8] 25000",\
"[9] 25000",\
"[10] 25000",\
"[11] 25000",\
"[12] 25000",\
"[13] 25000",\
"[14] 25000",\
"[15] 25000",\
"",\
"[.]",\
"acq_ctrl = BOOL : y",\
"",\
NULL }

#endif

#ifndef EXCL_SCALER

#define SCALER_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} SCALER_COMMON;

#define SCALER_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 2",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 33",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 121",\
"Period = INT : 100",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] ucntau-daq",\
"Frontend name = STRING : [32] Frontend",\
"Frontend file name = STRING : [256] CAENfrontend_v1730.cpp",\
"Status = STRING : [256] Frontend@ucntau-daq",\
"Status color = STRING : [32] #00FF00",\
"Hidden = BOOL : n",\
"",\
NULL }

#endif

#ifndef EXCL_FASTCOMTEC

#define LM87VAL_BANK_DEFINED

typedef struct {
  double    v25;
  double    v15;
  double    vcc;
  double    v50;
  double    v120;
  double    v33;
  double    ain2;
  INT       tfpga;
  INT       tboard;
  INT       fan1;
  INT       dac;
  struct {
    struct {
      struct {
        struct {
          INT       tfpga;
          INT       tboard;
        } lm87val;
      } variables;
    } fastcomtec;
  } equipment;
} LM87VAL_BANK;

#define LM87VAL_BANK_STR(_name) const char *_name[] = {\
"[.]",\
"v25 = DOUBLE : 0",\
"v15 = DOUBLE : 0",\
"vcc = DOUBLE : 0",\
"v50 = DOUBLE : 0",\
"v120 = DOUBLE : 0",\
"v33 = DOUBLE : 0",\
"ain2 = DOUBLE : 0",\
"tfpga = INT : 50",\
"tboard = INT : 50",\
"fan1 = INT : 0",\
"dac = INT : 0",\
"",\
"[Equipment/FastComTec/Variables/LM87VAL]",\
"tfpga = INT : 45",\
"tboard = INT : 44",\
"",\
NULL }

#define MCSD_BANK_DEFINED

typedef struct {
  INT       channel;
  double    time;
  INT       full;
  INT       tag;
  INT       edge;
  double    realtime;
} MCSD_BANK;

#define MCSD_BANK_STR(_name) const char *_name[] = {\
"[.]",\
"channel = INT : 1",\
"time = DOUBLE : 8411593250",\
"full = INT : 0",\
"tag = INT : 0",\
"edge = INT : 1",\
"realtime = DOUBLE : 6.7292746",\
"",\
NULL }

#define FASTCOMTEC_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} FASTCOMTEC_COMMON;

#define FASTCOMTEC_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 3",\
"Trigger mask = WORD : 65535",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 1",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 121",\
"Period = INT : 10",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] ucntau-daq",\
"Frontend name = STRING : [32] Frontend",\
"Frontend file name = STRING : [256] CAENfrontend_v1730.cpp",\
"Status = STRING : [256] Frontend@ucntau-daq",\
"Status color = STRING : [32] #00FF00",\
"Hidden = BOOL : n",\
"",\
NULL }

#define FASTCOMTEC_SETTINGS_DEFINED

typedef struct {
  struct {
    char      channel_1[32];
    char      channel_2[32];
    char      channel_3[32];
    char      channel_4[32];
    char      channel_5[32];
    char      channel_6[32];
    char      channel_7[32];
    char      channel_8[32];
    char      channel_9[32];
    char      channel_10[32];
    char      channel_11[32];
    char      channel_0[32];
  } channel_labels;
} FASTCOMTEC_SETTINGS;

#define FASTCOMTEC_SETTINGS_STR(_name) const char *_name[] = {\
"[Channel_Labels]",\
"channel_1 = STRING : [32] Dagger PMT 1",\
"channel_2 = STRING : [32] Dagger PMT 2",\
"channel_3 = STRING : [32] Multiplexed Monitors",\
"channel_4 = STRING : [32] Active Cleaner",\
"channel_5 = STRING : [32] Stand Pipe",\
"channel_6 = STRING : [32] MCA 2 Start",\
"channel_7 = STRING : [32] Old Monitor",\
"channel_8 = STRING : [32] Bare Monitor",\
"channel_9 = STRING : [32] GV Foil Monitor",\
"channel_10 = STRING : [32] ",\
"channel_11 = STRING : [32] ",\
"channel_0 = STRING : [32] MCA 1 Start",\
"",\
NULL }

#endif

#ifndef EXCL_FASTCOMTEC2

#define MCSD_BANK_DEFINED

typedef struct {
  INT       channel;
  double    time;
  INT       full;
  INT       tag;
  INT       edge;
  double    realtime;
} MCSD_BANK;

#define MCSD_BANK_STR(_name) const char *_name[] = {\
"[.]",\
"channel = INT : 2",\
"time = DOUBLE : 242835318937",\
"full = INT : 0",\
"tag = INT : 1",\
"edge = INT : 1",\
"realtime = DOUBLE : 194.2682551496",\
"",\
NULL }

#define FASTCOMTEC2_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} FASTCOMTEC2_COMMON;

#define FASTCOMTEC2_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 4",\
"Trigger mask = WORD : 65535",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 1",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : n",\
"Read on = INT : 121",\
"Period = INT : 101",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] ucntau-daq",\
"Frontend name = STRING : [32] Frontend",\
"Frontend file name = STRING : [256] CAENfrontend_v1730.cpp",\
"Status = STRING : [256] Frontend@ucntau-daq",\
"Status color = STRING : [32] #00FF00",\
"Hidden = BOOL : n",\
"",\
NULL }

#endif

#ifndef EXCL_FASTCOMTEC3

#define FASTCOMTEC3_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} FASTCOMTEC3_COMMON;

#define FASTCOMTEC3_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 4",\
"Trigger mask = WORD : 65535",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 1",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : n",\
"Read on = INT : 121",\
"Period = INT : 100",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] lifetimedaq2.lanl.gov",\
"Frontend name = STRING : [32] MCS_Frontend_203",\
"Frontend file name = STRING : [256] MCS6A_frontend.cpp",\
"Status = STRING : [256] MCS_Frontend_203@lifetimedaq2.lanl.gov",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"",\
NULL }

#endif

