// written by ashwani rathee

#include "perceptron.h"

#include <vector>
#include <cmath>

extern "C" {
#include "bp/bp.param.h"
#include "core.param.h"
#include "globals/assert.h"
#include "statistics.h"
}

#define DEBUG(proc_id, args...) _DEBUG(proc_id, DEBUG_BP_DIR, ##args)
#define INIT_BASE_VAL 0
#define BASE_HIST_LENGTH 9
#define BASE_PERCEPTRON_ENTRIES 170000
#define BASE_PERCEPTRON_HASH(addr) (addr % BASE_PERCEPTRON_ENTRIES)
#define PERCEPTRON_THRESHOLD (float)((1.93 * BASE_HIST_LENGTH) + 14)

namespace {

struct perceptron_State {
  std::vector<float> weights;
  perceptron_State() :
      weights(BASE_HIST_LENGTH, INIT_BASE_VAL) {
  } 
};

struct hist_perceptron_table {
  uns64 global_hist = 0;
  std::vector<perceptron_State> perceptron_state_all_cores;
};
std::vector<hist_perceptron_table> perceptron_hist;
}  // namespace

void bp_perceptron_timestamp(Op* op) {}
void bp_perceptron_recover(Recovery_Info* info) {}
void bp_perceptron_spec_update(Op* op) {}
void bp_perceptron_retire(Op* op) {}
uns8 bp_perceptron_full(uns proc_id) {
  return 0;
}

char* toBinaryString(const int n) {
  int num_bits = sizeof(int) * 8;
  char *string = (char*)malloc(num_bits + 1);
  if (!string) {
    return NULL;
  }
  int new_n = n;
  for (int i = num_bits - 1; i >= 0; i--) {
    string[i] = (new_n & 1) + '0';
    new_n = new_n >> 1;
  }
  string[num_bits] = '\0';
  return string;
}

void bp_perceptron_init() {
  perceptron_hist.resize(NUM_CORES);
  for (uns i = 0; i < NUM_CORES; i++) {
    perceptron_hist[i].perceptron_state_all_cores.resize(BASE_PERCEPTRON_ENTRIES);
    for (int j = 0; j < BASE_PERCEPTRON_ENTRIES; j++) {
      perceptron_hist[i].perceptron_state_all_cores[j] = perceptron_State();
    }
  }
}

uns8 bp_perceptron_pred(Op* op) {
  const uns   proc_id          = op->proc_id;
  const Addr  addr             = op->oracle_info.pred_addr;
  const uns64 hist             = op->oracle_info.pred_perceptron_global_hist;
  uns32       index            = BASE_PERCEPTRON_HASH(addr);
  const auto& perceptron_state = perceptron_hist.at(proc_id).perceptron_state_all_cores[index];
  float       prediction_score = 0.0f;
  uns64* saved_hist;
  int x_i;

  for(int i = 0; i < BASE_HIST_LENGTH; ++i) {
    int history_bit = ((hist >> i) & 0x1) ? 1 : -1;  // 1 -> +1, 0 -> -1
    // Bias weight is just the weight.
    if (i == 0) {
      prediction_score += perceptron_state.weights[i];
      continue;
    }
    prediction_score += ((history_bit * perceptron_state.weights[i]));
  }
  saved_hist = &perceptron_hist.at(proc_id).global_hist;
  *saved_hist >>= 1;
  if (op->oracle_info.dir == 1) {
    x_i = 1;
  } else if (op->oracle_info.dir == 0) {
    x_i = 0;
  } else {
    DEBUG(proc_id, "WEIRD VALUE! %d", op->oracle_info.dir);
    x_i = 0;
  }
  // Insert the newest result.
  *saved_hist |= ((uns64)x_i << 63);
  op->oracle_info.pred_perceptron_global_hist = *saved_hist;
  DEBUG(proc_id, "oracle_info.dir: (%d) hist: (%s)\n", x_i, toBinaryString(*saved_hist));

  op->base_perceptron_output = prediction_score;
  uns8 pred = (prediction_score >= 0.0f) ? 1 : 0;
  return pred;
}

void bp_perceptron_update(Op* op) {
  const uns   proc_id          = op->proc_id;
  const Addr  addr             = op->oracle_info.pred_addr;
  const uns64 hist             = op->oracle_info.pred_perceptron_global_hist;
  uns64       index            = BASE_PERCEPTRON_HASH(addr);
  auto&     perceptron_state   = perceptron_hist.at(proc_id).perceptron_state_all_cores[index];
  const uns8 actual_result     = op->oracle_info.dir;
  float base_perceptron_output = op->base_perceptron_output;
  int t;

  if(op->table_info->cf_type != CF_CBR) {
    // If op is not a conditional branch, we do not interact with perceptron.
    return;
  }
  if(actual_result == 1)
    t = 1;
  else
    t = -1;

  DEBUG(proc_id, "t: %d; calculated: %f; threshold: %f\n", t, base_perceptron_output, PERCEPTRON_THRESHOLD);

  if((base_perceptron_output <= 0 && t <= 0) || (abs(base_perceptron_output) <= PERCEPTRON_THRESHOLD)) {
    for(int i = 0; i < BASE_HIST_LENGTH; ++i) {
      int history_bit = ((hist >> i) & 0x1) ? 1 : -1;  // 1 -> +1, 0 -> -1
      //DEBUG(proc_id, "== hist_bit: %d; hist: (%s)\n", history_bit, toBinaryString(hist));
      // Bias weight is just the weight.
      if (i == 0) {
        perceptron_state.weights[i] += t;
        continue;
      }
      perceptron_state.weights[i] += history_bit * t;
    }
  }
}
