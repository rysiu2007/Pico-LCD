# Lightweight HD44780 LCD Driver for RP2040

A high-performance, **zero-dependency** C driver for character LCDs (16x2, 20x4) running on the Raspberry Pi Pico.

### Features:
* **4-bit Interface:** Optimized for GPIO-saving (requires only 6 pins).
* **Custom Print Engine:** Includes a lightweight `print()` implementation with support for formatted strings (`%d`, `%u`, `%f`) without the overhead of `stdio.h`.
* **Hardware-Specific Timings:** Precise `busy_wait` implementations based on the HD44780 datasheet for maximum stability.
* **Bare-metal Ready:** Just a single `.c` and `.h` file—easy to drop into any Pico SDK project.
