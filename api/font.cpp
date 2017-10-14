#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static stbtt_fontinfo guiRegularFont;
static bool fontRendererInitialised;

static void OSFontRendererInitialise() {
	if (fontRendererInitialised) {
		return;
	}

	OSHandle regularFontHandle = OSOpenNamedSharedMemory(OS_GUI_FONT_REGULAR, OSCStringLength(OS_GUI_FONT_REGULAR));

	if (regularFontHandle == OS_INVALID_HANDLE) {
		OSPrint("Could not get font handle.\n");
		return;
	}

	void *loadedFont = OSMapSharedMemory(regularFontHandle, 0, OS_SHARED_MEMORY_MAP_ALL);

	if (stbtt_InitFont(&guiRegularFont, (uint8_t *) loadedFont, 0)) {
		// The font was loaded.
	} else {
		return;
	}

	fontRendererInitialised = true;
}

static int MeasureStringWidth(char *string, size_t stringLength, float scale) {
	char *stringEnd = string + stringLength;
	int totalWidth = 0;

	OSFontRendererInitialise();
	if (!fontRendererInitialised) return -1;

	while (string < stringEnd) {
		int character = utf8_value(string);

		int advanceWidth, leftSideBearing;
		stbtt_GetCodepointHMetrics(&guiRegularFont, character, &advanceWidth, &leftSideBearing);
		advanceWidth    *= scale;
		leftSideBearing *= scale;

		totalWidth += advanceWidth;

		string = utf8_advance(string);
	}

	return totalWidth;
}

static float GetGUIFontScale() {
	OSFontRendererInitialise();
	if (!fontRendererInitialised) return OS_ERROR_COULD_NOT_LOAD_FONT;

#define FONT_SIZE (16.0f)
	float scale = stbtt_ScaleForPixelHeight(&guiRegularFont, FONT_SIZE);
	return scale;
}

OSError OSDrawString(OSHandle surface, OSRectangle region, 
		char *string, size_t stringLength,
		unsigned alignment, uint32_t color, int32_t backgroundColor) {
	if (!stringLength) return OS_SUCCESS;

	OSFontRendererInitialise();
	if (!fontRendererInitialised) return OS_ERROR_COULD_NOT_LOAD_FONT;

	float scale = GetGUIFontScale();

	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&guiRegularFont, &ascent, &descent, &lineGap);
	ascent  *= scale;
	descent *= scale;
	lineGap *= scale;

	int lineHeight = ascent - descent + lineGap;

	char *stringEnd = string + stringLength;

	OSLinearBuffer linearBuffer;
	OSGetLinearBuffer(surface, &linearBuffer);

	int totalWidth = MeasureStringWidth(string, stringLength, scale);

	OSPoint outputPosition;

	if (alignment & OS_DRAW_STRING_HALIGN_LEFT) {
		if (alignment & OS_DRAW_STRING_HALIGN_RIGHT) {
			// Centered text.
			outputPosition.x = (region.right - region.left) / 2 + region.left - totalWidth / 2;
		} else {
			// Left-justified text.
			outputPosition.x = region.left;
		}
	} else if (alignment & OS_DRAW_STRING_HALIGN_RIGHT) {
		// Right-justified text.
		outputPosition.x = region.right - totalWidth;
	} else	outputPosition.x = region.left;

	if (alignment & OS_DRAW_STRING_VALIGN_TOP) {
		if (alignment & OS_DRAW_STRING_VALIGN_BOTTOM) {
			// Centered text.
			outputPosition.y = (region.bottom - region.top) / 2 + region.top - lineHeight / 2 - 1;
		} else {
			// Top-justified text.
			outputPosition.y = region.top;
		}
	} else if (alignment & OS_DRAW_STRING_VALIGN_BOTTOM) {
		// Bottom-justified text.
		outputPosition.y = region.right - lineHeight;
	} else	outputPosition.y = region.top;

	outputPosition.y += ascent;

	OSRectangle invalidatedRegion = OSRectangle(outputPosition.x, outputPosition.x,
			outputPosition.y, outputPosition.y);

	while (string < stringEnd) {
		int character = utf8_value(string);

		int advanceWidth, leftSideBearing;
		stbtt_GetCodepointHMetrics(&guiRegularFont, character, &advanceWidth, &leftSideBearing);
		advanceWidth    *= scale;
		leftSideBearing *= scale;

		uint8_t *output;

		if (outputPosition.x + advanceWidth < region.left) {
			goto skipCharacter;
		} else if (outputPosition.x >= region.right) {
			break;
		}

		int ix0, iy0, ix1, iy1;
		stbtt_GetCodepointBitmapBox(&guiRegularFont, character, scale, scale, &ix0, &iy0, &ix1, &iy1);

		int width, height, xoff, yoff;
		output = stbtt_GetCodepointBitmap(&guiRegularFont, scale, scale, character, &width, &height, &xoff, &yoff);

		if (!output) {
			goto skipCharacter;
		}

		for (int y = 0; y < height; y++) {
			int oY = outputPosition.y + iy0 + y;

			if (oY < region.top) continue;
			if (oY >= region.bottom) break;

			if (oY < invalidatedRegion.top) invalidatedRegion.top = oY;
			if (oY > invalidatedRegion.bottom) invalidatedRegion.bottom = oY;

			for (int x = 0; x < width; x++) {
				int oX = outputPosition.x + ix0 + x;
			
				if (oX < region.left) continue;
				if (oX >= region.right) break;

				if (oX < invalidatedRegion.left) invalidatedRegion.left = oX;
				if (oX > invalidatedRegion.right) invalidatedRegion.right = oX;

				uint8_t pixel = output[x + y * width];
				uint32_t *destination = (uint32_t *) ((uint8_t *) linearBuffer.buffer + 
						(oX) * 4 + 
						(oY) * linearBuffer.stride);

				uint32_t sourcePixel = (pixel << 24) | color;

				if (pixel == 0xFF) {
					*destination = sourcePixel;
				} else {
					uint32_t original;

					if (backgroundColor < 0) {
						original = *destination;
					} else {
						original = backgroundColor;
					}

					uint32_t modified = sourcePixel;

					uint32_t alpha1 = (modified & 0xFF000000) >> 24;
					uint32_t alpha2 = 255 - alpha1;
					uint32_t r2 = alpha2 * ((original & 0x000000FF) >> 0);
					uint32_t g2 = alpha2 * ((original & 0x0000FF00) >> 8);
					uint32_t b2 = alpha2 * ((original & 0x00FF0000) >> 16);
					uint32_t r1 = alpha1 * ((modified & 0x000000FF) >> 0);
					uint32_t g1 = alpha1 * ((modified & 0x0000FF00) >> 8);
					uint32_t b1 = alpha1 * ((modified & 0x00FF0000) >> 16);
					uint32_t result = 0xFF000000 | (0x00FF0000 & ((b1 + b2) << 8)) 
						| (0x0000FF00 & ((g1 + g2) << 0)) 
						| (0x000000FF & ((r1 + r2) >> 8));

					*destination = result;
				}
			}
		}

		OSHeapFree(output);

		skipCharacter:
		outputPosition.x += advanceWidth;
		string = utf8_advance(string);
	}

	OSPrint("drawn string, invalidating: %d, %d, %d, %d\n", 
			invalidatedRegion.left, invalidatedRegion.right,
			invalidatedRegion.top, invalidatedRegion.bottom);
	OSInvalidateRectangle(surface, invalidatedRegion);

	return OS_SUCCESS;
}
