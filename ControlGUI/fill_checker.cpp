#include "fill_checker.h"

fill_checker::fill_checker()
{

}

fill_checker::~fill_checker()
{

}

bool fill_checker::inspect_fill()
{
 /*   if(hmgx_tagbit_data.size() < 4) {
                   return;
           }
           std::vector<double> hmgx_hits;
           for(auto it = hmgx_tagbit_data.begin(); it < hmgx_tagbit_data.end()-1; it++) {
                   if(it->state() == 1 && ((it+1)->realtime)-it->realtime) > 0.1 { //If it's a rising edge and high for at least 100ms, say that's an H-GX hit
                           hmgx_hits.push_back(it->realtime);
                   }
           }

           ExpFill fillFunc();
           double avgInterarrival;
           for(auto it = hmgx_hits.begin(); it < hmgx_hits.end()-1; it++) {
                   fillFunc.addOffset(*it);
                   avgInterarrival += *it;
           }

           avgInterarrival = avgInterarrival / (hmgx_hits.size()-1);

           if(avgInterarrival > 7.0 && hmgx_hits.size() < 6) { //want 3 from the tail to measure for 10s interarrival
                   return;
           }
           else if(avgInterarrival < 7.0 && hmgx_hits.size() < 8) { //want 3 from the tail to measure for 5s interarrival
                   return;
           }

           int endTime = int(floor(hmgx_hits.back()-1));
           TH1D spHist("fillHist", "fillHist", endTime, 0, endTime);
           for(int i = 0; i < endTime; i++) {
                   spHist.SetBinContent(i+1, mcs6a1_data[5].ct[i]);
           }
           TF1 fit("fit", fillFunc, 0.0, endTime, fillFunc.getNumOffsets());

           for(int i = 0; i < fillFunc.getNumOffsets(); i++) {
                   fit->SetParameter(i, 800);
                   fit->SetParLimits(i, 0.0, 10000);
           }

           spHist.Fit(fit);

           std::vector<double> pulseHeights;
           for(int i = 0; i < fillFunc.getNumOffsets(); i++) {
                   pulseHeights.push_back(fit->GetParameter(i));
           }

           std::vector<double> tailPulses;
           if(avgInterarrival > 7.0) {
                   tailPulses = std::vector<double>(pulseHeights.begin() + 2, pulseHeights.end());
           }
           else if(avgInterarrival < 7.0) {
                   tailPulses = std::vector<double>(pulseHeights.begin() + 4, pulseHeights.end());
           }

           double mFull = calc_Mscore(pulseHeights);
           double mTail = calc_Mscore(tailPulses);

           if(mFull > 20 || mTail > 7) {
               return false;
               //zmq_send(socket_check_fill, "bad", 3, 0);
           }

           return true;*/
}


double fill_checker::calc_Mscore(std::vector<double> pulseHeights)
{
   /* double med = std::nth_element(pulseHeights.begin(), pulseHeights.begin() + pulseHeights.size()/2, pulseHeights.end());
    std::vector<double> deviations;
    for(auto it = pulseHeights.begin(), it < pulseHeights.end(), it++) {
            deviations.push_back(fabs(*it-med));
    }

    double mad = std::nth_element(deviations.begin(), deviations.begin() + deviations.size()/2, deviations.end());

    if(mad == 0.0 or mad > 0.1 * med) {
            return -1.0;
    }

    double m_max = 0.0;
    for(auto it = deviations.begin(); it < deviations.end(); it++) {
            m_max = m_max < fabs(0.6745*(amp-med)/mad) ? fabs(0.6745*(amp-med)/mad) : m_max;
    }

    return m_max;*/
}
