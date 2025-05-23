/* Copyright 2020 HPS/SAFARI Research Groups
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/***************************************************************************************
 * File         : decode_stage.h
 * Author       : HPS Research Group
 * Date         : 2/17/1999
 * Description  :
 ***************************************************************************************/

#ifndef __DECODE_STAGE_H__
#define __DECODE_STAGE_H__

#include "stage_data.h"

/**************************************************************************************/
/* Types */

typedef struct Decode_Stage_struct {
  uns proc_id;
  Stage_Data* sds;     /* stage interface data (dynamically allocated number of pipe stages) */
  Stage_Data* last_sd; /* pointer to last decode pipeline stage (for passing ops to map) */
} Decode_Stage;

/**************************************************************************************/
/* External Variables */

extern Decode_Stage* dec;

/**************************************************************************************/
/* Prototypes */

/* vanilla hps model */
void set_decode_stage(Decode_Stage*);
void init_decode_stage(uns8, const char*);
void reset_decode_stage(void);
void recover_decode_stage(void);
void debug_decode_stage(void);
void update_decode_stage(Stage_Data*);
// Needed when ops skip the decode stage when fetched from the uop cache.
void decode_stage_process_op(Op*);

/**************************************************************************************/

#endif /* #ifndef __DECODE_STAGE_H__ */
