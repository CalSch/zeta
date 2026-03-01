#include "front_rl.h"
#include <pthread.h>
#include "emu.h"

pthread_t thread;

#define COMBINE_KEYS(a,b,c,d,e,f,g,h) ( \
	IsKeyDown(a) << 7 | \
	IsKeyDown(b) << 6 | \
	IsKeyDown(c) << 5 | \
	IsKeyDown(d) << 4 | \
	IsKeyDown(e) << 3 | \
	IsKeyDown(f) << 2 | \
	IsKeyDown(g) << 1 | \
	IsKeyDown(h) << 0 \
	)
u8 get_kb_row(u8 row) {
	switch (row) {
	case 0:
		return COMBINE_KEYS(KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F,KEY_G,KEY_H);
	case 1:
		return COMBINE_KEYS(KEY_I,KEY_J,KEY_K,KEY_L,KEY_M,KEY_N,KEY_O,KEY_P);
	case 2:
		return COMBINE_KEYS(KEY_Q,KEY_R,KEY_S,KEY_T,KEY_U,KEY_V,KEY_W,KEY_X);
	case 3:
		return COMBINE_KEYS(KEY_Y,KEY_Z,KEY_ZERO,KEY_ONE,KEY_TWO,KEY_THREE,KEY_FOUR,KEY_FIVE);
	case 4:
		return COMBINE_KEYS(KEY_SIX,KEY_SEVEN,KEY_EIGHT,KEY_NINE,KEY_LEFT_SHIFT,KEY_LEFT_CONTROL,KEY_LEFT_ALT,KEY_ESCAPE); // SIX SEVENNNN LETS GOOO
	case 5:
		return COMBINE_KEYS(KEY_BACKSPACE,KEY_DELETE,KEY_ENTER,KEY_TAB,KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT);
	case 6:
		return COMBINE_KEYS(KEY_MINUS,KEY_EQUAL,KEY_LEFT_BRACKET,KEY_RIGHT_BRACKET,KEY_BACKSLASH,KEY_SEMICOLON,KEY_APOSTROPHE,KEY_COMMA);
	case 7:
		return COMBINE_KEYS(KEY_PERIOD,KEY_SLASH,KEY_GRAVE,KEY_SPACE,0,0,0,0);
	default:
		return 0;
	}
}

void* thread_task(void* arg) {
	SetTraceLogLevel(LOG_WARNING);
	InitWindow(640,480,"mmmmm puter...");
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	Font font = LoadFont("PrintChar21.ttf");
	RenderTexture2D scr = LoadRenderTexture(640,480);

	SetTargetFPS(60);

	while (running && !WindowShouldClose()) {
		/* BeginDrawing(); */
		BeginTextureMode(scr);

		ClearBackground(BLACK);

		for (int r=0;r<TXT_ROWS;r++) {
			for (int c=0;c<TXT_COLS;c++) {
				char chr = vram.state.txt.text[r*TXT_COLS + c];
				DrawTextCodepoint(
					font,
					chr,
					(Vector2){
						c*8,
						r*8
					},
					8,
					WHITE
				);
			}
		}

		DrawFPS(10,10);

		EndTextureMode();

		Z80INT(&ctx,2);

		BeginDrawing();
		DrawTexturePro(
            scr.texture,
            (Rectangle) {
                0,
                0,
                (float)scr.texture.width,
                (float)-scr.texture.height
            },
            (Rectangle) {0,0,GetScreenWidth(),GetScreenHeight()},
            (Vector2) { 0, 0 },
            0,
            WHITE
        );

		EndDrawing();
	}

	CloseWindow();
	running = false; // stop the emulator TODO: replace with function
	return NULL;
}

void start_raylib() {
	pthread_create(&thread, NULL, thread_task, NULL);

}
void end_raylib() {
}
