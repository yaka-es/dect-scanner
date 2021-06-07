#ifndef DECT2_COMMON_H
#define DECT2_COMMON_H

#define A_FIELD_BITS		64
#define B_FIELD_BITS		320

#define MAX_PARTS		8			// Maximum number of DECT parts to be tracked
#define SMPL_BUF_LEN		(32 * 4)
#define TIME_TOL		10			// Time tolerance
#define INTER_SLOT_TIME		(480 * 4)
#define INTER_FRAME_TIME	(INTER_SLOT_TIME * 24)
#define S_FIELD_BITS		32
#define P32_D_FIELD_BITS	388
#define RFP_SYNC_FIELD		0xAAAAE98A

#endif // DECT2_COMMON_H
