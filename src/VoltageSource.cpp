// Copyright (c) 2020 Johannes Delport
// This code is licensed under MIT license (see LICENSE for details)

#include "JoSIM/VoltageSource.hpp"
#include "JoSIM/Misc.hpp"
#include "JoSIM/Errors.hpp"
#include "JoSIM/Constants.hpp"

#include <utility>

using namespace JoSIM;

VoltageSource::VoltageSource(
    const std::pair<tokens_t, string_o> &s, const NodeConfig &ncon,
    const nodemap &nm, std::unordered_set<std::string> &lm,
    nodeconnections &nc, int &bi, const int &si) {
  // Check if the label has already been defined
  if(lm.count(s.first.at(0)) != 0) {
    Errors::invalid_component_errors(
      ComponentErrors::DUPLICATE_LABEL, s.first.at(0));
  }
  // Set the label
  netlistInfo.label_ = s.first.at(0);
  // Add the label to the known labels list
  lm.emplace(s.first.at(0));
  // Set the node configuration type
  indexInfo.nodeConfig_ = ncon;
  // Set current index and increment it
  indexInfo.currentIndex_ = bi++;
  // Set te node indices, using token 2 and 3
  set_node_indices(tokens_t(s.first.begin()+1, s.first.begin()+3), nm, nc);
  // Set the non zero, column index and row pointer vectors
  set_matrix_info();
    // Since there are no current branch entries on main diagonal
  matrixInfo.columnIndex_.pop_back();
  matrixInfo.rowPointer_.back()--;
  // Set the source index
  sourceIndex_ = si;
}