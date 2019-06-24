#ifndef FILL_CHECKER_H
#define FILL_CHECKER_H

#include <stdlib.h>
#include <vector>

class fill_checker
{
public:
    fill_checker();
    ~fill_checker();

    bool inspect_fill();
    double calc_Mscore(std::vector<double> pulseHeights);

};

#endif // FILL_CHECKER_H
