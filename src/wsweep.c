//   Copyright 2022 Will Thomas
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0;
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct wl_display* disp;
struct wl_registry* reg;
struct wl_shm* shm;
struct xdg_wm_base* sh;
struct wl_compositor* comp;
struct wl_seat* seat;

struct wl_surface* srfc;
struct wl_callback* cb;
struct wl_buffer* bfr;
struct xdg_surface* xrfc;
struct xdg_toplevel* top;
struct wl_keyboard* kb;
struct wl_pointer* ms;

uint8_t* img;
uint8_t* pixl;
uint8_t* f;
uint16_t m;
uint8_t w;
uint8_t h;
uint8_t g = 1;
uint8_t cls = 0;

uint8_t mx;
uint8_t my;

void field_init() {
	srand(img);
	for (uint16_t n = m; n;) {
		uint8_t x = rand();
		uint8_t y = rand();
		if (x < w && y < h && *(f + (y * w) + x) != 9) {
			*(f + (y * w) + x) = 9;
			n--;
			
			if (x > 0 && *(f + (y * w) + (x - 1)) != 9) {
				*(f + (y * w) + (x - 1)) += 1;
			}
			if (y > 0 && *(f + ((y - 1) * w) + x) != 9) {
				*(f + ((y - 1) * w) + x) += 1;
			}
			if (x + 1 < w && *(f + (y * w) + (x + 1)) != 9) {
				*(f + (y * w) + (x + 1)) += 1;
			}
			if (y + 1 < h && *(f + ((y + 1) * w) + x) != 9) {
				*(f + ((y + 1) * w) + x) += 1;
			}
			if (x > 0 && y > 0 && *(f + ((y - 1) * w) + (x - 1)) != 9) {
				*(f + ((y - 1) * w) + (x - 1)) += 1;
			}
			if (y > 0 && x + 1 < w && *(f + ((y - 1) * w) + (x + 1)) != 9) {
				*(f + ((y - 1) * w) + (x + 1)) += 1;
			}
			if (y + 1 < h && x > 0 && *(f + ((y + 1) * w) + (x - 1)) != 9) {
				*(f + ((y + 1) * w) + (x - 1)) += 1;
			}
			if (y + 1 < h && x + 1 < w && *(f + ((y + 1) * w) + (x + 1)) != 9) {
				*(f + ((y + 1) * w) + (x + 1)) += 1;
			}
		}
	}
}

void field_test(uint8_t x, uint8_t y) {
	if (*(f + (y * w) + x) == 0) {
		*(f + (y * w) + x) += 32;
	}
	else if ((*(f + (y * w) + x) & 48) == 0) {
		*(f + (y * w) + x) += 32;
		return;
	}
	else {
		return;
	}
	
	uint8_t a = 0;
	if (x > 0 && (*(f + (y * w) + (x - 1)) & 48) == 16) {
		a += 1;
	}
	if (y > 0 && (*(f + ((y - 1) * w) + x) & 48) == 16) {
		a += 1;
	}
	if (x + 1 < w && (*(f + (y * w) + (x + 1)) & 48) == 16) {
		a += 1;
	}
	if (y + 1 < h && (*(f + ((y + 1) * w) + x) & 48) == 16) {
		a += 1;
	}
	if (x > 0 && y > 0 && (*(f + ((y - 1) * w) + (x - 1)) & 48) == 16) {
		a += 1;
	}
	if (x > 0 && y + 1 < h && (*(f + ((y + 1) * w) + (x - 1)) & 48) == 16) {
		a += 1;
	}
	if (x + 1 < w && y > 0 && (*(f + ((y - 1) * w) + (x + 1)) & 48) == 16) {
		a += 1;
	}
	if (x + 1 < w && y + 1 < h && (*(f + ((y + 1) * w) + (x + 1)) & 48) == 16) {
		a += 1;
	}
	
	if ((*(f + (y * w) + x) & 15) != 0 || (*(f + (y * w) + x) & 15) != a) {
		return;
	}
	
	if (x > 0) {
		field_test(x - 1, y);
	}
	if (y > 0) {
		field_test(x, y - 1);
	}
	if (x + 1 < w) {
		field_test(x + 1, y);
	}
	if (y + 1 < h) {
		field_test(x, y + 1);
	}
	if (x > 0 && y > 0) {
		field_test(x - 1, y - 1);
	}
	if (x > 0 && y + 1 < h) {
		field_test(x - 1, y + 1);
	}
	if (x + 1 < w && y > 0) {
		field_test(x + 1, y - 1);
	}
	if (x + 1 < w && y + 1 < h) {
		field_test(x + 1, y + 1);
	}
}

void field_lose() {
	g = 0;
	for (uint8_t y = 0; y < h; y++) {
		for (uint8_t x = 0; x < w; x++) {
			if (*(f + (y * w) + x) == 9) {
				*(f + (y * w) + x) = 41;
			}
			else if ((*(f + (y * w) + x) & 48) == 16 && (*(f + (y * w) + x) & 15) != 9) {
				*(f + (y * w) + x) = 48;
			}
		}
	}
}

uint64_t str_int(int8_t* a) {
	uint64_t b = 0;
	for(uint8_t i = 0; i < 20; i++) {
		if (a[i] == 0 || a[i] == ')') {
			return b;
		}
		b *= 10;
		if (a[i] == '1') {
			b += 1;
		}
		else if (a[i] == '2') {
			b += 2;
		}
		else if (a[i] == '3') {
			b += 3;
		}
		else if (a[i] == '4') {
			b += 4;
		}
		else if (a[i] == '5') {
			b += 5;
		}
		else if (a[i] == '6') {
			b += 6;
		}
		else if (a[i] == '7') {
			b += 7;
		}
		else if (a[i] == '8') {
			b += 8;
		}
		else if (a[i] == '9') {
			b += 9;
		}
		else if (a[i] != '0' && a[i] != ')') {
			return -1;
		}
	}
}

int32_t alc_shm(uint64_t sz) {
	int8_t name[8];
	name[0] = '/';
	name[7] = 0;
	for (uint8_t i = 1; i < 6; i++) {
		name[i] = (rand() & 23) + 97;
	}
	
	int32_t fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH);
	shm_unlink(name);
	ftruncate(fd, sz);

	return fd;
}

uint8_t* bmp_read(int8_t* path) {
	int32_t fd = open(path, O_RDONLY);
	if (fd == -1) {
		return 0;
	}
	
	struct stat fs;
	fstat(fd, &fs);
	
	uint8_t* data = mmap(0, fs.st_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	
	if (*data != 66 || *(data + 1) != 77) {
		return 0;
	}
	
	uint16_t off = *(data + 10) + (*(data + 11) << 8) + (*(data + 12) << 16) + (*(data + 13) << 24);
	uint16_t w = *(data + 18) + (*(data + 19) << 8) + (*(data + 20) << 16) + (*(data + 21) << 24);
	uint16_t h = *(data + 22) + (*(data + 23) << 8) + (*(data + 24) << 16) + (*(data + 25) << 24);
	uint16_t bpp = *(data + 28) + (*(data + 29) << 8);
	
	fd = alc_shm(w * h * 4);
	uint8_t* img = mmap(0, w * h * 4, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	for (uint16_t y = 0; y < h; y++) {
		for (uint16_t x = 0; x < w; x++) {
			*(img + (y * w * 4) + (x * 4) + 0) = *(data + off + 0);
			*(img + (y * w * 4) + (x * 4) + 1) = *(data + off + 1);
			*(img + (y * w * 4) + (x * 4) + 2) = *(data + off + 2);
			*(img + (y * w * 4) + (x * 4) + 3) = *(data + off + 3);
			off += 4;
		}
	}
	
	munmap(data, fs.st_size);
	return img;
}

uint8_t str_cmp(int8_t* a, int8_t* b) {
	for (uint32_t i = 0; 1; i++) {
		if (a[i] == 0 && b[i] == 0) {
			return 1;
		}
		else if (a[i] != b[i]) {
			return 0;
		}
	}
}

void resz() {
	int32_t fd = alc_shm((w * 20) * (h * 20) * 4);

	pixl = mmap(0, (w * 20) * (h * 20) * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	struct wl_shm_pool* pool = wl_shm_create_pool(shm, fd, (w * 20) * (h * 20) * 4);
	bfr = wl_shm_pool_create_buffer(pool, 0, w * 20, h * 20, w * 80, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(pool);
	close(fd);
}


void draw() {
	for (uint16_t y = 0; y < h * 20; y++) {
		for (uint16_t x = 0; x < w * 20; x++) {
			uint16_t z;
			if ((*(f + ((y / 20) * w) + (x / 20)) & 48) == 0) {
				z = 220;
			}
			else if ((*(f + ((y / 20) * w) + (x / 20)) & 48) == 16) {
				z = 180;
			}
			else if ((*(f + ((y / 20) * w) + (x / 20)) & 48) == 48) {
				z = 200;
			}
			else if (*(f + ((y / 20) * w) + (x / 20)) == 41) {
				z = 240;
			}
			else if ((*(f + ((y / 20) * w) + (x / 20)) & 48) == 32) {
				z = (*(f + ((y / 20) * w) + (x / 20)) & 15) * 20;
			}
			
			*(pixl + (y * w * 80) + (x * 4) + 0) = *(img + ((19 - (y % 20)) * 260 * 4) + (((x % 20) + z) * 4) + 0);
			*(pixl + (y * w * 80) + (x * 4) + 1) = *(img + ((19 - (y % 20)) * 260 * 4) + (((x % 20) + z) * 4) + 1);
			*(pixl + (y * w * 80) + (x * 4) + 2) = *(img + ((19 - (y % 20)) * 260 * 4) + (((x % 20) + z) * 4) + 2);
			*(pixl + (y * w * 80) + (x * 4) + 3) = *(img + ((19 - (y % 20)) * 260 * 4) + (((x % 20) + z) * 4) + 3);
		}
	}
	
	wl_surface_attach(srfc, bfr, 0, 0);
	wl_surface_damage_buffer(srfc, 0, 0, w * 20, h * 20);
	wl_surface_commit(srfc);
}

struct wl_callback_listener cb_list;

void frame_new(void* data, struct wl_callback* cb, uint32_t a) {
	wl_callback_destroy(cb);
	cb = wl_surface_frame(srfc);
	wl_callback_add_listener(cb, &cb_list, 0);
	draw();
}

struct wl_callback_listener cb_list = {
	.done = frame_new
};

void xrfc_conf(void* data, struct xdg_surface* xrfc, uint32_t ser) {
	xdg_surface_ack_configure(xrfc, ser);
	resz();
	draw();
}

struct xdg_surface_listener xrfc_list = {
	.configure = xrfc_conf
};

void top_conf(void* data, struct xdg_toplevel* top, int32_t nw, int32_t nh, struct wl_array* stat) {
	
}

void top_cls(void* data, struct xdg_toplevel* top) {
	cls = 1;
}

struct xdg_toplevel_listener top_list = {
	.configure = top_conf,
	.close = top_cls
};

void sh_ping(void* data, struct xdg_wm_base* sh, uint32_t ser) {
	xdg_wm_base_pong(sh, ser);
}

struct xdg_wm_base_listener sh_list = {
	.ping = sh_ping
};

void kb_map(void* data, struct wl_keyboard* kb, uint32_t frmt, int32_t fd, uint32_t sz) {
	
}

void kb_enter(void* data, struct wl_keyboard* kb, uint32_t ser, struct wl_surface* srfc, struct wl_array* keys) {
	
}

void kb_leave(void* data, struct wl_keyboard* kb, uint32_t ser, struct wl_surface* srfc) {
	
}

void kb_key(void* data, struct wl_keyboard* kb, uint32_t ser, uint32_t t, uint32_t key, uint32_t stat) {
	if (key == 1) {
		cls = 1;
	}
}

void kb_mod(void* data, struct wl_keyboard* kb, uint32_t ser, uint32_t dep, uint32_t lat, uint32_t lock, uint32_t grp) {
	
}

void kb_rep(void* data, struct wl_keyboard* kb, int32_t rate, int32_t del) {
	
}

struct wl_keyboard_listener kb_list = {
	.keymap = kb_map,
	.enter = kb_enter,
	.leave = kb_leave,
	.key = kb_key,
	.modifiers = kb_mod,
	.repeat_info = kb_rep
};

void ms_enter(void* data, struct wl_pointer* ms, uint32_t ser, struct wl_surface* srfc, wl_fixed_t x, wl_fixed_t y) {
	
}

void ms_leave(void* data, struct wl_pointer* ms, uint32_t ser, struct wl_surface* srfc) {
	
}

void ms_motion(void* data, struct wl_pointer* ms, uint32_t t, wl_fixed_t x, wl_fixed_t y) {
	mx = wl_fixed_to_int(x) / 20;
	my = wl_fixed_to_int(y) / 20;
}

void ms_button(void* data, struct wl_pointer* ms, uint32_t ser, uint32_t t, uint32_t but, uint32_t stat) {
	if (!g) {
		return;
	}
	if (but == 272 && !stat && *(f + (my * w) + mx) == 9) {
		field_lose();
	}
	else if (but == 272 && !stat && (*(f + (my * w) + mx) & 16) == 0) {
		field_test(mx, my);
	}
	else if (but == 273 && !stat && (*(f + (my * w) + mx) & 48) == 0) {
		*(f + (my * w) + mx) += 16;
	}
	else if (but == 273 && !stat && (*(f + (my * w) + mx) & 48) == 16) {
		*(f + (my * w) + mx) -= 16;
	}
}

void ms_axis(void* data, struct wl_pointer* ms, uint32_t t, uint32_t axis, wl_fixed_t val) {
	
}

void ms_frame(void* data, struct wl_pointer* ms) {
	
}

void ms_axis_source(void* data, struct wl_pointer* ms, uint32_t src) {
	
}

void ms_axis_stop(void* data, struct wl_pointer* ms, uint32_t t, uint32_t axis) {
	
}

void ms_axis_discrete(void* data, struct wl_pointer* ms, uint32_t axis, int32_t dis) {
	
}

struct wl_pointer_listener ms_list = {
	.enter = ms_enter,
	.leave = ms_leave,
	.motion = ms_motion,
	.button = ms_button,
	.axis = ms_axis,
	.frame = ms_frame,
	.axis_source = ms_axis_source,
	.axis_stop = ms_axis_stop,
	.axis_discrete = ms_axis_discrete,
};

void seat_cap(void* data, struct wl_seat* seat, uint32_t cap) {
	
}

void seat_name(void* data, struct wl_seat* seat, int8_t* name) {
		
}

struct wl_seat_listener seat_list = {
	.capabilities = seat_cap,
	.name = seat_name
};

void reg_glob(void* data, struct wl_registry* reg, uint32_t name, int8_t* intf, uint32_t v) {
	if (str_cmp(intf, wl_compositor_interface.name)) {
		comp = wl_registry_bind(reg, name, &wl_compositor_interface, 4);
	}
	else if (str_cmp(intf, wl_shm_interface.name)) {
		shm = wl_registry_bind(reg, name, &wl_shm_interface, 1);
	}
	else if (str_cmp(intf, xdg_wm_base_interface.name)) {
		sh = wl_registry_bind(reg, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(sh, &sh_list, 0);
	}
	else if (str_cmp(intf, wl_seat_interface.name)) {
		seat = wl_registry_bind(reg, name, &wl_seat_interface, 1);
		wl_seat_add_listener(seat, &seat_list, 0);
	}
}

void reg_glob_rem(void* data, struct wl_registry* reg, uint32_t name) {
	
}

struct wl_registry_listener reg_list = {
	.global = reg_glob,
	.global_remove = reg_glob_rem
};

int8_t main(int32_t argc, int8_t** argv) {
	w = 16;
	h = 8;
	m = 24;
	
	if (argc == 4) {
		w = str_int(argv[1]);
		h = str_int(argv[2]);
		m = str_int(argv[3]);
	}
	
	img = bmp_read("img/atlas.bmp");
	f = mmap(0, w * h * 4, PROT_WRITE | PROT_READ, MAP_SHARED, alc_shm(w * h * 4), 0);
	field_init();
	
	disp = wl_display_connect(0);
	reg = wl_display_get_registry(disp);
	wl_registry_add_listener(reg, &reg_list, 0);
	wl_display_roundtrip(disp);
	
	srfc = wl_compositor_create_surface(comp);
	cb = wl_surface_frame(srfc);
	wl_callback_add_listener(cb, &cb_list, 0);
	
	xrfc = xdg_wm_base_get_xdg_surface(sh, srfc);
	xdg_surface_add_listener(xrfc, &xrfc_list, 0);
	top = xdg_surface_get_toplevel(xrfc);
	xdg_toplevel_add_listener(top, &top_list, 0);
	//xdg_toplevel_set_title(top, "wayland client");
	xdg_toplevel_set_min_size(top, w, h);
	xdg_toplevel_set_max_size(top, w, h);
	wl_surface_commit(srfc);
	
	kb = wl_seat_get_keyboard(seat);
	wl_keyboard_add_listener(kb, &kb_list, 0);
	ms = wl_seat_get_pointer(seat);
	wl_pointer_add_listener(ms, &ms_list, 0);
	
	while (!cls) {
		wl_display_dispatch(disp);
	}
	
	wl_keyboard_destroy(kb);
	wl_pointer_destroy(ms);
	xdg_toplevel_destroy(top);
	xdg_surface_destroy(xrfc);
	wl_surface_destroy(srfc);
	
	wl_seat_release(seat);
	wl_display_disconnect(disp);
	
	munmap(f, w * h * 4);
	munmap(img, 20800);
	return 0;
}
