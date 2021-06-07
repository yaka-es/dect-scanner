/* -*- c++ -*- */
/*
 * Copyright 2015 Pavel Yazev <pyazev@gmail.com>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_DECT2_PACKET_DECODER_IMPL_H
#define INCLUDED_DECT2_PACKET_DECODER_IMPL_H

#include "dect2_common.h"
#include "packet_decoder.h"

namespace gr {
namespace dect2 {

class packet_decoder_impl : public packet_decoder
{
private:
	typedef enum {
		_RFP_,	// Radio Fixed Part
		_PP_,	// Portable Part
	} part_type;

	typedef struct part_descriptor_item {
		bool active;
		bool voice_present;
		bool log_update;
		bool part_id_rcvd;
		bool qt_rcvd;

		bool rfp_fn_cor; // set true if frame number was corrected from RFP part

		uint8_t frame_number;
		uint64_t rx_seq;
		uint8_t part_id[5];
		part_type type;

		uint64_t packet_cnt;
		uint64_t afield_bad_crc_cnt;

		struct part_descriptor_item *pair;
	} part_descriptor_item;

	part_descriptor_item d_part_descriptor[MAX_PARTS];
	part_descriptor_item *d_cur_part;
	uint32_t d_selected_rx_id;

	void *part_updated_callback_arg;
	part_updated_callback_t part_updated_callback;

	void *part_lost_callback_arg;
	part_lost_callback_t part_lost_callback;

	uint32_t decode_afield(uint8_t *field_data);

	int calculate_output_stream_length(const gr_vector_int &ninput_items);
	void msg_event_handler(pmt::pmt_t msg);

	void print_parts(void);

	void emit_part_updated(uint32_t rx_id);
	void emit_part_lost(uint32_t rx_id);

public:
	packet_decoder_impl();
	virtual ~packet_decoder_impl();

	virtual void select_rx_part(uint32_t rx_id);

	int work(int noutput_items,
		gr_vector_int &ninput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items);

	virtual void clear_parts(void);
	virtual void set_part_updated_callback(part_updated_callback_t callback, void *arg);
	virtual void set_part_lost_callback(part_lost_callback_t callback, void *arg);
};

} // namespace dect2
} // namespace gr

#endif /* INCLUDED_DECT2_PACKET_DECODER_IMPL_H */
