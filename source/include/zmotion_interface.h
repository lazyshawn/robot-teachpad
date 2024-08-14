

#include <eigen3/Eigen/Dense>
#include <vector>

class ZauxRobot {
public:
  int handle_ = -1;

public:
  ZauxRobot();

  int connect();
  int disconnect();

  int get_axis_param(const std::vector<int>& axisList, char* paramName, std::vector<float>& paramList);
  int set_axis_param(const std::vector<int>& axisList, char* paramName, const std::vector<float>& paramList, int principal = -1);

  int jog_moving(int axis, int type);

};

std::vector<double> get_uniform_double(int size);

