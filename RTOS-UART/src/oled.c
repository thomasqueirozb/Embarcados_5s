#include <asf.h>
void pin_toggle(Pio *pio, uint32_t mask) {
	if (pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio, mask);
}