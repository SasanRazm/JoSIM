// Copyright (c) 2019 Johannes Delport
// This code is licensed under MIT license (see LICENSE for details)
#ifndef JOSIM_JJ_HPP
#define JOSIM_JJ_HPP

#include "./ParameterName.hpp"
#include "./Parameters.hpp"

#include <vector>
#include <unordered_map>
#include <optional>

class JJ {
  private:
    std::string label_;
    std::vector<double> nonZeros_;
    std::vector<int> columnIndex_;
    std::optional<int> posIndex_, negIndex_;
    int currentIndex_;
    int variableIndex_;
    double value_;
  public:
    JJ() {};
    
    static void create_jj(
        const std::pair<std::string, std::string> &s,
        std::vector<JJ> &jjs, 
        const std::unordered_map<std::string, int> &nm, 
        std::vector<int> &nc,
        const std::unordered_map<JoSIM::ParameterName, Parameter> &p,
        const int &antyp,
        const double &timestep);
    void set_label(const std::string &l) { label_ = l; }
    void set_nonZeros_and_columnIndex(const std::pair<std::string, std::string> &n, const std::unordered_map<std::string, int> &nm, const std::string &s, std::vector<int> &nc);
    void set_indices(const std::pair<std::string, std::string> &n, const std::unordered_map<std::string, int> &nm, std::vector<int> &nc);
    void set_currentIndex(const int &cc) { currentIndex_ = cc; }
    void set_variableIndex(const int &vc) { variableIndex_ = vc; }
    void set_value(const std::pair<std::string, std::string> &s, 
        const std::unordered_map<JoSIM::ParameterName, Parameter> &p,
        const int &antyp, const double &timestep);

    std::string get_label() const { return label_; }
    std::vector<double> get_nonZeros() const { return nonZeros_; }
    std::vector<int> get_columnIndex() const { return columnIndex_; }
    std::optional<int> get_posIndex() const { return posIndex_; }
    std::optional<int> get_negIndex() const { return negIndex_; }
    int get_currentIndex() const { return currentIndex_; }
    int get_variableIndex() const { return variableIndex_; }
    double get_value() const { return value_; }

};

#endif