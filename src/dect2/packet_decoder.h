/* -*- c++ -*- */
/*
 * Copyright 2015 <+YOU OR YOUR COMPANY+>.
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


#ifndef INCLUDED_DECT2_PACKET_DECODER_H
#define INCLUDED_DECT2_PACKET_DECODER_H

#include <gnuradio/tagged_stream_block.h>

#include "api.h"

namespace gr {
namespace dect2 {

/*!
 * \brief <+description of block+>
 * \ingroup dect2
 *
 */
class DECT2_API packet_decoder : virtual public gr::tagged_stream_block
{
public:
	typedef boost::shared_ptr<packet_decoder> sptr;

	/*!
	 * \brief Return a shared_ptr to a new instance of dect2::packet_decoder.
 	 *
	 * To avoid accidental use of raw pointers, dect2::packet_decoder's
	 * constructor is in a private implementation
	 * class. dect2::packet_decoder::make is the public interface for
	 * creating new instances.
	 */
	static sptr make();

	virtual void select_rx_part(uint32_t rx_id) = 0;

	typedef struct {
		uint32_t rx_id;
		uint8_t part_id[5];
		bool is_fixed_part;
		bool voice_present;
	} part_info_t;

	typedef void (*part_updated_callback_t)(void *arg, const part_info_t *part_info);
	typedef void (*part_lost_callback_t)(void *arg, const part_info_t *part_info);

	virtual void clear_parts(void) = 0;
	virtual void set_part_updated_callback(part_updated_callback_t callback, void *arg) = 0;
	virtual void set_part_lost_callback(part_lost_callback_t callback, void *arg) = 0;
};

} // namespace dect2
} // namespace gr

#endif /* INCLUDED_DECT2_PACKET_DECODER_H */
