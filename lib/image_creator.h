struct color
{
	int red;
	int green;
	int blue;
};

struct ascii_character
{
	char brightness;
	struct color color;
};

int create_image_from_ascii(struct ascii_character** character, int image_height, int image_width, char* output);