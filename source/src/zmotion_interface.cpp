
#include "zmotion_interface.h"
#include <random>
#include <chrono>
#include <iostream>

ZauxRobot::ZauxRobot() {
}

int ZauxRobot::connect() {
  auto flag = get_uniform_double(1);
  std::cout << "flag = " << flag[0] << std::endl;
  if (flag[0] > 0.4) {
    handle_ = std::floor(flag[0]);
    return 0;
  }
  return -1;
}

int ZauxRobot::disconnect() {
  handle_ = -1;
  return 0;
}

int ZauxRobot::get_axis_param(const std::vector<int>& axisList, char* paramName, std::vector<float>& paramList) {
  return 0;
}

int ZauxRobot::set_axis_param(const std::vector<int>& axisList, char* paramName, const std::vector<float>& paramList, int principal) {
  return 0;
}

int ZauxRobot::jog_moving(int axis, int type) {
  // 运动结束
  if (type == 0) {
    std::cout << "move end" << std::endl;
  }
  // 正向运动
  else if (type > 0){
    std::cout << "move forward" << std::endl;
  }
  // 负向运动
  else {
    std::cout << "move backward" << std::endl;
  }
  return 0;
}

std::vector<double> get_uniform_double(int size) {
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_real_distribution<double> uniform(0, 1);

  std::vector<double> randomVector(size);
  for (int i=0; i<size; ++i) {
    randomVector[i] = uniform(generator);
  }
  return randomVector;
}

