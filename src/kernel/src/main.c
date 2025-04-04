#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "font.h"
#include "limine.h"

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8
#define CHAR_SPACING 1

// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((
    used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((
    used,
    section(
        ".limine_requests"))) static volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used,
               section(".limine_requests_"
                       "start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((
    used,
    section(
        ".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n) {
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  for (size_t i = 0; i < n; i++) {
    pdest[i] = psrc[i];
  }

  return dest;
}

void *memset(void *s, int c, size_t n) {
  uint8_t *p = (uint8_t *)s;

  for (size_t i = 0; i < n; i++) {
    p[i] = (uint8_t)c;
  }

  return s;
}

void *memmove(void *dest, const void *src, size_t n) {
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  if (src > dest) {
    for (size_t i = 0; i < n; i++) {
      pdest[i] = psrc[i];
    }
  } else if (src < dest) {
    for (size_t i = n; i > 0; i--) {
      pdest[i - 1] = psrc[i - 1];
    }
  }

  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t *p1 = (const uint8_t *)s1;
  const uint8_t *p2 = (const uint8_t *)s2;

  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] < p2[i] ? -1 : 1;
    }
  }

  return 0;
}

// Halt and catch fire function.
static void hcf(void) { asm("hlt"); }

const char *snoopy_ascii[] = {
    "  ,-~~-.___.                                             ",
    " / |  '     \\         It was a dark and stormy night....",
    "(  )         0                                           ",
    " \\_/-, ,----'                                            ",
    "    ====           //                                    ",
    "   /  \\-'~;    /~~~(O)                                  ",
    "  /  __/~|   /       |                                   ",
    "=(  _____| (_________|   W<                              "};

// Draw a single character at (x, y)
void draw_char(uint32_t *fb, uint32_t pitch, uint32_t color, char c, size_t x,
               size_t y) {
  if (c < 0 || c > 127)
    return;
  for (size_t row = 0; row < CHAR_HEIGHT; row++) {
    uint8_t bits = font_data[(int)c][row];
    for (size_t col = 0; col < CHAR_WIDTH; col++) {
      if (bits & (1 << (7 - col))) {
        fb[(y + row) * (pitch / 4) + (x + col)] = color;
      }
    }
  }
}

// Draw string
void draw_string(uint32_t *fb, uint32_t pitch, uint32_t color, const char *str,
                 size_t x, size_t y) {
  while (*str) {
    draw_char(fb, pitch, color, *str, x, y);
    x += CHAR_WIDTH + CHAR_SPACING;
    str++;
  }
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED == false) {
    hcf();
  }

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1) {
    hcf();
  }

  // Fetch the first framebuffer.
  struct limine_framebuffer *framebuffer =
      framebuffer_request.response->framebuffers[0];

  uint32_t *fb_ptr = (uint32_t *)framebuffer->address;

  // Clear screen to black
  for (size_t y = 0; y < framebuffer->height; y++) {
    for (size_t x = 0; x < framebuffer->width; x++) {
      fb_ptr[y * (framebuffer->pitch / 4) + x] = 0x000000;
    }
  }

  // Draw ASCII art
  for (size_t i = 0; i < sizeof(snoopy_ascii) / sizeof(snoopy_ascii[0]); i++) {
    draw_string(fb_ptr, framebuffer->pitch, 0xFFFFFF, snoopy_ascii[i], 20,
                20 + i * CHAR_HEIGHT);
  }

  // We're done, just hang...
  hcf();
}
