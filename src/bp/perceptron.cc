// written by ashwani rathee

#include "perceptron.h"

#include <vector>

extern "C" {
#include "bp/bp.param.h"
#include "core.param.h"
#include "globals/assert.h"
#include "statistics.h"
}

#define DEBUG(proc_id, args...) _DEBUG(proc_id, DEBUG_BP_DIR, ##args)
#define INIT_BASE_VAL 0
#define LEARNING_RATE 0.01

namespace {

struct perceptron_State {
  std::vector<float> weights;
  perceptron_State() :
      weights(64, INIT_BASE_VAL) {
  } 
};


std::vector<perceptron_State> perceptron_state_all_cores;

}  // namespace

void bp_perceptron_timestamp(Op* op) {}
void bp_perceptron_recover(Recovery_Info* info) {}
void bp_perceptron_spec_update(Op* op) {}
void bp_perceptron_retire(Op* op) {}
uns8 bp_perceptron_full(uns proc_id) {
  return 0;
}


void bp_perceptron_init() {
  perceptron_state_all_cores.resize(NUM_CORES);
}

uns8 bp_perceptron_pred(Op* op) {
  const uns   proc_id          = op->proc_id;
  const auto& perceptron_state = perceptron_state_all_cores.at(proc_id);

  const uns64 hist             = op->oracle_info.pred_perceptron_global_hist;
  float       prediction_score = 0.0f;
  for(int i = 0; i < 64; ++i) {
    int history_bit = ((hist >> i) & 0x1) ? 1 : -1;  // 1 -> +1, 0 -> -1
    prediction_score += ((history_bit * perceptron_state.weights[i]));
  }

  uns8 pred = (prediction_score >= 0.0f) ? 1 : 0;
  return pred;
}

void bp_perceptron_update(Op* op) {
  const uns proc_id          = op->proc_id;  // Core ID
  auto&     perceptron_state = perceptron_state_all_cores.at(
    proc_id);  // Perceptron state for this core

  const uns32 hist = op->oracle_info.pred_perceptron_global_hist;
  const uns8  pred = op->oracle_info.pred;
  const uns8 actual = op->oracle_info.dir;

  if(pred != actual) {
    for(int i = 0; i < 64; ++i) {
      int history_bit = ((hist >> i) & 0x1) ? 1 : -1;  // 1 -> +1, 0 -> -1
      if(actual == 1) {
        perceptron_state.weights[i] += history_bit * LEARNING_RATE;
      } else { 
        perceptron_state.weights[i] -= history_bit * LEARNING_RATE;
      }
    }
  }
}
