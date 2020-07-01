// Copyright (c) 2020 Johannes Delport
// This code is licensed under MIT license (see LICENSE for details)

#include "JoSIM/TransmissionLine.hpp"
#include "JoSIM/IntegrationType.hpp"
#include "JoSIM/Misc.hpp"
#include "JoSIM/Errors.hpp"
#include "JoSIM/Constants.hpp"

#include <iostream>
#include <string>
#include <algorithm>
#include <locale>
#include <functional>

using namespace JoSIM;

TransmissionLine::TransmissionLine(
  const std::pair<tokens_t, string_o> &s, const NodeConfig &ncon, 
  const std::optional<NodeConfig> &ncon2, const nodemap &nm, 
  std::unordered_set<std::string> &lm, nodeconnections &nc, 
  const param_map &pm, const AnalysisType &at, const double &h, int &bi) {
  // Check if the label has already been defined
  if(lm.count(s.first.at(0)) != 0) {
    Errors::invalid_component_errors(
      ComponentErrors::DUPLICATE_LABEL, s.first.at(0));
  }
  // Set the label
  netlistInfo.label_ = s.first.at(0);
  // Add the label to the known labels list
  lm.emplace(s.first.at(0));
  // Find the parts that contain the impedance and time delay
  for(int i = 5; i < s.first.size(); ++i) {
    // Impedance
    if(::memcmp(s.first.at(i).c_str(), "Z0=", 3) == 0) {
      // If impedance keyword is specified but no value given
      if(s.first.at(i).length() < 4) {
        // Complain
        Errors::invalid_component_errors(
          ComponentErrors::INVALID_TX_DEFINED, 
            Misc::vector_to_string(s.first));
      }
      // Set the value (Z0), this should be a value
      netlistInfo.value_ = parse_param(s.first.at(i).substr(3), pm, s.second);
      // If not a value
      if(isnan(netlistInfo.value_)) {
        // Complain
        Errors::invalid_component_errors(
          ComponentErrors::INVALID_TX_DEFINED, 
            Misc::vector_to_string(s.first));
      }
    }
    // Time Delay
    if(::memcmp(s.first.at(i).c_str(), "TD=", 3) == 0) {
      // If impedance keyword is specified but no value given
      if(s.first.at(i).length() < 4) {
        // Complain
        Errors::invalid_component_errors(
          ComponentErrors::INVALID_TX_DEFINED, 
            Misc::vector_to_string(s.first));
      }
      // Set the time delay (TD), this should be a value
      timestepDelay_ = parse_param(s.first.at(i).substr(3), pm, s.second) / h;
      // If not a value
      if(isnan(timestepDelay_)) {
        // Complain
        Errors::invalid_component_errors(
          ComponentErrors::INVALID_TX_DEFINED, 
            Misc::vector_to_string(s.first));
      }
    }
  }
  // Set the node configuration type
  indexInfo.nodeConfig_ = ncon;
  // Set current index and increment it
  indexInfo.currentIndex_ = bi++;
  // Set second current index and increment it
  currentIndex2_ = bi++;
  // Set te node indices, using token 2 and 3
  set_node_indices(tokens_t(s.first.begin()+1, s.first.begin()+3), nm, nc);
  // Set the node configuration type for secondary nodes
  nodeConfig2_ = ncon2.value();
  // Set te node indices, using token 4 and 5
  set_secondary_node_indices(
    tokens_t(s.first.begin()+3, s.first.begin()+5), nm, nc);
  // Set the non zero, column index and row pointer vectors
  set_matrix_info();
  // Append the value to the non zero vector
  if (at == AnalysisType::Voltage) { 
    // If voltage mode analysis then append -Z0
    matrixInfo.nonZeros_.emplace_back(-netlistInfo.value_);
  } else if (at == AnalysisType::Phase) { 
    // If phase mdoe analysis then append -(2*h/3) * (Z0/σ)
    matrixInfo.nonZeros_.emplace_back(
      -(2.0*h/3.0) * (netlistInfo.value_ / Constants::SIGMA));
  }
  // Set the non zero, column index and row pointer vectors
  set_secondary_matrix_info();
  // Append the value to the non zero vector
  if (at == AnalysisType::Voltage) { 
    // If voltage mode analysis then append -Z0
    matrixInfo.nonZeros_.emplace_back(-netlistInfo.value_);
  } else if (at == AnalysisType::Phase) { 
    // If phase mdoe analysis then append -(2*h/3) * (Z0/σ)
    matrixInfo.nonZeros_.emplace_back(
      -(2.0*h/3.0) * (netlistInfo.value_ / Constants::SIGMA));
  }
}

void TransmissionLine::set_secondary_node_indices(
  const tokens_t &t, const nodemap &nm, nodeconnections &nc) {
  // Transmission lines have 4 nodes, this sets the 2nd pair of the 4
  switch(nodeConfig2_) {
  case NodeConfig::POSGND:
    posIndex2_ = nm.at(t.at(0));
    nc.at(nm.at(t.at(0))).emplace_back(std::make_pair(1, currentIndex2_));
    break;
  case NodeConfig::GNDNEG:
    negIndex2_ = nm.at(t.at(1));
    nc.at(nm.at(t.at(1))).emplace_back(std::make_pair(-1, currentIndex2_));
    break;
  case NodeConfig::POSNEG:
    posIndex2_ = nm.at(t.at(0));
    negIndex2_ = nm.at(t.at(1));
    nc.at(nm.at(t.at(0))).emplace_back(std::make_pair(1, currentIndex2_));
    nc.at(nm.at(t.at(1))).emplace_back(std::make_pair(-1, currentIndex2_));
    break;
  case NodeConfig::GND:
    break;
  }
}

void TransmissionLine::set_secondary_matrix_info() {
  // Tranmission lines have two current branches, this sets the second branch
  switch(nodeConfig2_) {
  case NodeConfig::POSGND:
    matrixInfo.nonZeros_.emplace_back(1);
    matrixInfo.columnIndex_.emplace_back(posIndex2_.value());
    matrixInfo.rowPointer_.emplace_back(2);
    break;
  case NodeConfig::GNDNEG:
    matrixInfo.nonZeros_.emplace_back(-1);
    matrixInfo.columnIndex_.emplace_back(negIndex2_.value());
    matrixInfo.rowPointer_.emplace_back(2);
    break;
  case NodeConfig::POSNEG:
    matrixInfo.nonZeros_.emplace_back(1);
    matrixInfo.nonZeros_.emplace_back(-1);
    matrixInfo.columnIndex_.emplace_back(posIndex2_.value());
    matrixInfo.columnIndex_.emplace_back(negIndex2_.value());
    matrixInfo.rowPointer_.emplace_back(3);
    break;
  case NodeConfig::GND:
    matrixInfo.rowPointer_.emplace_back(1);
    break;
  }
  matrixInfo.columnIndex_.emplace_back(currentIndex2_);
}
