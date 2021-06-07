/* main.cxx */
/*
 * Copyright 2021 Alexander Samarin <sasha.devel@gmail.com>
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

#define USE_OSMOSDR	1
#define USE_UHD		0

#include <sys/types.h>

#include <getopt.h>
#include <stdio.h>
#include <unistd.h>

#include <gnuradio/basic_block.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fractional_resampler_cc.h>
#include <gnuradio/filter/rational_resampler_base_ccc.h>
#include <gnuradio/filter/rational_resampler_base_fff.h>
#include <gnuradio/tagged_stream_block.h>
#include <gnuradio/thread/thread.h>
#include <gnuradio/top_block.h>
#if USE_OSMOSDR
#include <osmosdr/device.h>
#include <osmosdr/source.h>
#endif
#if USE_UHD
#include <gnuradio/uhd/usrp_source.h>
#endif

#include "dect2/packet_decoder.h"
#include "dect2/packet_receiver.h"
#include "dect2/phase_diff.h"
#include "logging.h"

using gr::filter::rational_resampler_base_ccc;
using gr::filter::rational_resampler_base_fff;
using gr::filter::fractional_resampler_cc;
using gr::blocks::null_sink;

static volatile bool g_application_running;

class console_dumper : virtual public gr::tagged_stream_block {
public:
	typedef boost::shared_ptr<console_dumper> sptr;
	static sptr make();
};

class console_dumper_impl : public console_dumper {
public:
	console_dumper_impl();
	~console_dumper_impl();
private:
	void msg_event_handler(pmt::pmt_t msg);
	virtual int work(int noutput_items,
		gr_vector_int &ninput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items);
};

console_dumper::sptr console_dumper::make()
{
	return gnuradio::get_initial_sptr(new console_dumper_impl());
}

console_dumper_impl::console_dumper_impl() :
	gr::tagged_stream_block(
		"console_dumper",
		gr::io_signature::make(0, 0, 0),
		gr::io_signature::make(0, 0, 0), std::string("packet_len"))
{
	message_port_register_in(pmt::mp("in"));
	set_msg_handler(pmt::mp("in"), boost::bind(&console_dumper_impl::msg_event_handler, this, _1));
}

console_dumper_impl::~console_dumper_impl()
{
}

void console_dumper_impl::msg_event_handler(pmt::pmt_t msg)
{
#if 0
	pmt::pmt_t value = pmt::dict_ref(msg, pmt::mp("log_msg"), pmt::mp(""));
	std::string s = symbol_to_string(value);
	if (s != "") {
		//fprintf(stderr, "%s\n", s.c_str());
	}
#else
	(void)msg;
#endif
}

int console_dumper_impl::work(int noutput_items,
	gr_vector_int &ninput_items,
	gr_vector_const_void_star &input_items,
	gr_vector_void_star &output_items)
{
	(void)ninput_items;
	(void)input_items;
	(void)output_items;
	return noutput_items;
}

static double dect_symbol_rate = 1152000;
static double dect_occupied_bandwidth = 1.2 * dect_symbol_rate;
static double dect_channel_bandwidth = 1.728e6;
static double baseband_sampling_rate = 3200000;
static double rx_gain = 30;
static double rx_freq = 1890432000;
static int rx_freq_index = 0;

// static int part_id = 0;

#define DECT_CHANNELS 10
static double _rx_freq_options[DECT_CHANNELS] = {
	1881792000, // 0
	1883520000, // 1
	1885248000, // 2
	1886876000, // 3
	1888704000, // 4
	1890432000, // 5
	1892160000, // 6
	1893888000, // 7
	1895616000, // 8
	1897344000, // 9
};

static gr::top_block_sptr tb;
#if USE_OSMOSDR
static osmosdr::source::sptr source;
#endif
#if USE_UHD
static gr::uhd::usrp_source::sptr source;
#endif
static gr::dect2::packet_decoder::sptr packet_decoder;

static void part_updated_handler(void *arg, const gr::dect2::packet_decoder::part_info_t *part_info)
{
	printf("scan-report: U %d %8.6lf %u %02x%02x%02x%02x%02x %c %c\n",
		rx_freq_index, _rx_freq_options[rx_freq_index] / 1e6, part_info->rx_id,
		part_info->part_id[0],
		part_info->part_id[1],
		part_info->part_id[2],
		part_info->part_id[3],
		part_info->part_id[4],
		part_info->is_fixed_part ? 'F' : 'P',
		part_info->voice_present ? 'V' : '-');
}

static void part_lost_handler(void *arg, const gr::dect2::packet_decoder::part_info_t *part_info)
{
	printf("scan-report: L %d %8.6lf %u %02x%02x%02x%02x%02x %c %c\n",
		rx_freq_index, _rx_freq_options[rx_freq_index] / 1e6, part_info->rx_id,
		part_info->part_id[0],
		part_info->part_id[1],
		part_info->part_id[2],
		part_info->part_id[3],
		part_info->part_id[4],
		part_info->is_fixed_part ? 'F' : 'P',
		part_info->voice_present ? 'V' : '-');
}

static const char options[] = "a:v";
static struct option long_options[] = {
	{ "help", 0, NULL, 0 },
	{ "usage", 0, NULL, 0 },
	{ "version", 0, NULL, 0 },
	{ "verbose", 0, NULL, 'v' },
	{ "device-args", 1, NULL, 'a' },
	{ NULL, 0, NULL, 0 },
};

static void print_help(const char *argv0)
{
	fprintf(stderr, "%s {--help|--usage|--version}\n", argv0);
	fprintf(stderr, "%s {-a|--device-args} args\n", argv0);
}

static void print_version()
{
	fprintf(stderr, "1.0\n");
}

int main(int argc, char **argv)
{
	const char *argv0 = argv[0];

	std::string device_args = "bladerf=0"; // "hackrf=0";

	for (;;) {
		const char *option_name = NULL;
		int option_index = 0;
		int c = getopt_long(argc, argv, options, long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 0:
			option_name = long_options[option_index].name;

			if (strcmp(option_name, "help") == 0) {
				print_help(argv0);
				return EXIT_SUCCESS;

			} else if (strcmp(option_name, "usage") == 0) {
				print_help(argv0);
				return EXIT_SUCCESS;

			} else if (strcmp(option_name, "version") == 0) {
				print_version();
				return EXIT_SUCCESS;

			} else {
				if (optarg)
					log_error("unknown option --%s=\"%s\"\n", option_name, optarg);
				else
					log_error("unknown option --%s\n", option_name);
			}
			break;

		case 'a':
			device_args = optarg;
			break;

		case 'v':
			loglevel++;
			break;

		case '?':
			return EXIT_FAILURE;

		default:
			return EXIT_FAILURE;
		}
	}

	log_info("device arguments: \"%s\"\n", device_args.c_str());

	tb = gr::make_top_block("dect_scanner");

	rx_freq = _rx_freq_options[4];

#if USE_OSMOSDR

#if 0
	osmosdr::devices_t devs = osmosdr::device::find();
	for (auto it = devs.begin(); it != devs.end(); it++) {
		std::string s = it->to_pp_string();
		printf("OsmoSDR: found device:\n%s\n", s.c_str());
	}
#endif

	source = osmosdr::source::make(device_args);

	double samp_rate = source->set_sample_rate(baseband_sampling_rate);
	log_info("Actual sample rate: %5.3lf Hz\n", samp_rate);

	double center_freq = source->set_center_freq(rx_freq, 0);
	log_info("Actual central frequency: %5.3lf MHz\n", center_freq / 1.0e6);

	std::vector<std::string> gain_names = source->get_gain_names(0);
	for (auto it : gain_names) {
		osmosdr::gain_range_t gain_range = source->get_gain_range(it, 0);
		log_info("Found gain: %s min %lf max %lf step %lf\n", it.c_str(), gain_range.start(), gain_range.stop(), gain_range.step());
	}

	double gain = source->set_gain(rx_gain, 0);
	log_info("Actual gain: %5.3lf\n", gain);

	//source->set_antenna("TX/RX", 0);

	std::vector<std::string> antennas = source->get_antennas(0);
	for (auto it : antennas) {
		log_info("Found antenna: %s\n", it.c_str());
	}
	std::string ant = source->set_antenna("RX1", 0);
	log_info("Using antenna %s\n", ant.c_str());
#endif

#if USE_UHD
	std::string device_addr = "";

	source = gr::uhd::usrp_source::make(device_addr, uhd::stream_args_t("fc32"));

	source->set_samp_rate(baseband_sampling_rate);
	source->set_center_freq(rx_freq, 0);
	source->set_gain(rx_gain, 0);
	source->set_antenna("RX2", 0);
	// source->set_auto_dc_offset(true, 0);
	// source->set_auto_iq_balance(true, 0);
#endif

	double bw = source->get_bandwidth(0);
	log_info("Bandwidth: %5.3lf MHz\n", bw);

	std::vector<float> taps0;

	rational_resampler_base_fff::sptr rational_resampler_xxx_0 = rational_resampler_base_fff::make(6, 1, taps0);

	std::vector<float> resampler_filter_taps_float = gr::filter::firdes::low_pass_2(
		1, 3 * baseband_sampling_rate, dect_occupied_bandwidth / 2, (dect_channel_bandwidth - dect_occupied_bandwidth) / 2, 30);
	std::vector<gr_complex> resampler_filter_taps;
	resampler_filter_taps.resize(resampler_filter_taps_float.size());
	for (size_t i = 0; i < resampler_filter_taps_float.size(); i++)
		resampler_filter_taps[i] = resampler_filter_taps_float[i];

	rational_resampler_base_ccc::sptr rational_resampler = rational_resampler_base_ccc::make(3, 2, resampler_filter_taps);

	fractional_resampler_cc::sptr fractional_resampler = fractional_resampler_cc::make(0, float((3.0 * baseband_sampling_rate / 2.0) / dect_symbol_rate / 4.0));

	gr::dect2::phase_diff::sptr phase_diff =
		gr::dect2::phase_diff::make();

	gr::dect2::packet_receiver::sptr packet_receiver =
		gr::dect2::packet_receiver::make();

	packet_decoder = gr::dect2::packet_decoder::make();
	packet_decoder->set_part_updated_callback(part_updated_handler, nullptr);
	packet_decoder->set_part_lost_callback(part_lost_handler, nullptr);

	console_dumper::sptr console_0 = console_dumper::make();

	null_sink::sptr null_sink_1 = null_sink::make(1);

#if USE_UHD
	tb->connect(source, 0, rational_resampler, 0);
#endif
#if USE_OSMOSDR
	tb->connect(source, 0, rational_resampler, 0);
#endif
	tb->connect(rational_resampler, 0, fractional_resampler, 0);
	tb->connect(fractional_resampler, 0, phase_diff, 0);
	tb->connect(phase_diff, 0, packet_receiver, 0);
	tb->connect(packet_receiver, 0, packet_decoder, 0);
	tb->msg_connect(packet_decoder, "log_out", console_0, "in");
	tb->msg_connect(packet_receiver, "rcvr_msg_out", packet_decoder, "rcvr_msg_in");

	tb->connect(packet_decoder, 0, null_sink_1, 0);

	g_application_running = true;
	while (g_application_running) {
		try {
			tb->start(1);

			usleep(100000);

			rx_freq_index = (rx_freq_index == (DECT_CHANNELS - 1)) ? 0 : (rx_freq_index + 1);
			rx_freq = _rx_freq_options[rx_freq_index];

			log_debug("DECT channel %d, frequency %5.3lf MHz\n", rx_freq_index, rx_freq / 1e6);

			tb->stop();
			source->set_center_freq(rx_freq, 0);
			packet_receiver->reset();
			packet_decoder->clear_parts();

		} catch (std::runtime_error &ex) {
			tb->stop();
			tb->wait();
			log_error("catched std::runtime_error(\"%s\"), restarting...\n", ex.what());
		} catch (std::exception &ex) {
			tb->stop();
			tb->wait();
			log_error("catched std::exception(\"%s\"), restarting...\n", ex.what());
		} catch (...) {
			tb->stop();
			tb->wait();
			log_error("catched other exception, restarting...\n");
		}
	}

	return 0;
}
