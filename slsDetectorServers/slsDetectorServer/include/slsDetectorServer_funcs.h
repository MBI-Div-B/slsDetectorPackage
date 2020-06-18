#pragma once
#include "clogger.h"
#include "sls_detector_defs.h"

enum numberMode { DEC, HEX };
#define GOODBYE (-200)
#define REBOOT  (-400)

// initialization functions
int printSocketReadError();
void init_detector();
int decode_function(int);
const char *getRetName();
void function_table();
void functionNotImplemented();
void modeNotImplemented(char *modename, int mode);
void validate(int arg, int retval, char *modename, enum numberMode nummode);
void validate64(int64_t arg, int64_t retval, char *modename,
                enum numberMode nummode);
int executeCommand(char *command, char *result, enum TLogLevel level);
int M_nofunc(int);
#if defined(MYTHEN3D) || defined(GOTTHARD2D)
void rebootNiosControllerAndFPGA();
#endif

// functions called by client
int exec_command(int);
int get_detector_type(int);
int get_external_signal_flag(int);
int set_external_signal_flag(int);
int set_timing_mode(int);
int get_firmware_version(int);
int get_server_version(int);
int get_serial_number(int);
int set_firmware_test(int);
int set_bus_test(int);
int set_image_test_mode(int);
int get_image_test_mode(int);
int set_dac(int);
int get_adc(int);
int write_register(int);
int read_register(int);
int set_module(int);
int set_settings(int);
int get_threshold_energy(int);
int start_acquisition(int);
int stop_acquisition(int);
int get_run_status(int);
int start_and_read_all(int);
int read_all(int);
int get_num_frames(int);
int set_num_frames(int);
int get_num_triggers(int);
int set_num_triggers(int);
int get_num_additional_storage_cells(int);
int set_num_additional_storage_cells(int);
int get_num_analog_samples(int);
int set_num_analog_samples(int);
int get_num_digital_samples(int);
int set_num_digital_samples(int);
int get_exptime(int);
int set_exptime(int);
int get_period(int);
int set_period(int);
int get_delay_after_trigger(int);
int set_delay_after_trigger(int);
int get_sub_exptime(int);
int set_sub_exptime(int);
int get_sub_deadtime(int);
int set_sub_deadtime(int);
int get_storage_cell_delay(int);
int set_storage_cell_delay(int);
int get_frames_left(int);
int get_triggers_left(int);
int get_exptime_left(int);
int get_period_left(int);
int get_delay_after_trigger_left(int);
int get_measured_period(int);
int get_measured_subperiod(int);
int get_frames_from_start(int);
int get_actual_time(int);
int get_measurement_time(int);
int set_dynamic_range(int);
int set_roi(int);
int get_roi(int);
int lock_server(int);
int get_last_client_ip(int);
int set_port(int);
int calibrate_pedestal(int);
int enable_ten_giga(int);
int set_all_trimbits(int);
int set_pattern_io_control(int);
int set_pattern_clock_control(int);
int set_pattern_word(int);
int set_pattern_loop_addresses(int);
int set_pattern_loop_cycles(int);
int set_pattern_wait_addr(int);
int set_pattern_wait_time(int);
int set_pattern_mask(int);
int get_pattern_mask(int);
int set_pattern_bit_mask(int);
int get_pattern_bit_mask(int);
int write_adc_register(int);
int set_counter_bit(int);
int pulse_pixel(int);
int pulse_pixel_and_move(int);
int pulse_chip(int);
int set_rate_correct(int);
int get_rate_correct(int);
int set_ten_giga_flow_control(int);
int get_ten_giga_flow_control(int);
int set_transmission_delay_frame(int);
int get_transmission_delay_frame(int);
int set_transmission_delay_left(int);
int get_transmission_delay_left(int);
int set_transmission_delay_right(int);
int get_transmission_delay_right(int);
int program_fpga(int);
int reset_fpga(int);
int power_chip(int);
int set_activate(int);
int prepare_acquisition(int);
int threshold_temp(int);
int temp_control(int);
int temp_event(int);
int auto_comp_disable(int);
int storage_cell_start(int);
int check_version(int);
int software_trigger(int);
int led(int);
int digital_io_delay(int);
int copy_detector_server(int);
int reboot_controller(int);
int set_adc_enable_mask(int);
int get_adc_enable_mask(int);
int set_adc_invert(int);
int get_adc_invert(int);
int set_external_sampling_source(int);
int set_external_sampling(int);
int set_starting_frame_number(int);
int get_starting_frame_number(int);
int set_quad(int);
int get_quad(int);
int set_interrupt_subframe(int);
int get_interrupt_subframe(int);
int set_read_n_lines(int);
int get_read_n_lines(int);
void calculate_and_set_position();
int set_detector_position(int);
int check_detector_idle();
int is_configurable();
void configure_mac();
int set_source_udp_ip(int);
int get_source_udp_ip(int);
int set_source_udp_ip2(int);
int get_source_udp_ip2(int);
int set_dest_udp_ip(int);
int get_dest_udp_ip(int);
int set_dest_udp_ip2(int);
int get_dest_udp_ip2(int);
int set_source_udp_mac(int);
int get_source_udp_mac(int);
int set_source_udp_mac2(int);
int get_source_udp_mac2(int);
int set_dest_udp_mac(int);
int get_dest_udp_mac(int);
int set_dest_udp_mac2(int);
int get_dest_udp_mac2(int);
int set_dest_udp_port(int);
int get_dest_udp_port(int);
int set_dest_udp_port2(int);
int get_dest_udp_port2(int);
int set_num_interfaces(int);
int get_num_interfaces(int);
int set_interface_sel(int);
int get_interface_sel(int);
int set_parallel_mode(int);
int get_parallel_mode(int);
int set_overflow_mode(int);
int get_overflow_mode(int);
int set_readout_mode(int);
int get_readout_mode(int);
int set_clock_frequency(int);
int get_clock_frequency(int);
int set_clock_phase(int);
int get_clock_phase(int);
int get_max_clock_phase_shift(int);
int set_clock_divider(int);
int get_clock_divider(int);
int set_pipeline(int);
int get_pipeline(int);
int set_on_chip_dac(int);
int get_on_chip_dac(int);
int set_inject_channel(int);
int get_inject_channel(int);
int set_veto_photon(int);
int get_veto_photon(int);
int set_veto_reference(int);
int get_burst_mode(int);
int set_burst_mode(int);
int set_adc_enable_mask_10g(int);
int get_adc_enable_mask_10g(int);
int set_counter_mask(int);
int get_counter_mask(int);
int get_num_bursts(int);
int set_num_bursts(int);
int get_burst_period(int);
int set_burst_period(int);
int get_current_source(int);
int set_current_source(int);
int get_timing_source(int);
int set_timing_source(int);
int get_num_channels(int);
int update_rate_correction(int);
int get_receiver_parameters(int);
int start_pattern(int);
int set_num_gates(int);
int get_num_gates(int);
int set_gate_delay(int);
int get_gate_delay(int);
int get_exptime_all_gates(int);
int get_gate_delay_all_gates(int);
int get_veto(int);
int set_veto(int);