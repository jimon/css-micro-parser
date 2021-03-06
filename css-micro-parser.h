// CSS micro parser for fb css-layout

#pragma once

#include <css-layout.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

bool css_parse_is_ws(char c) {return c == ' ' || c == '\n' || c == '\r';}
bool css_parse_is_fsep(char c) {return c == ':' || c == '\0' || css_parse_is_ws(c);}
bool css_parse_is_vsep(char c) {return c == ';' || c == '\0' || css_parse_is_ws(c);}
size_t css_skip_ws(const char * t, size_t i)
{
	while(css_parse_is_ws(t[i])) ++i;
	return i;
}
void css_parse_shuffle6(float * values, float * new_values)
{
	// css provide top right bottom left start end
	for(size_t i = 0; i < 6; ++i)
		new_values[i] = NAN;

	if(values[0] == NAN)
		return;
	else if(values[1] == NAN)
		new_values[CSS_LEFT] = new_values[CSS_RIGHT] = new_values[CSS_BOTTOM] = new_values[CSS_TOP] = values[0];
	else if(values[2] == NAN)
	{
		new_values[CSS_BOTTOM] = new_values[CSS_TOP] = values[0];
		new_values[CSS_LEFT] = new_values[CSS_RIGHT] = values[1];
	}
	else if(values[3] == NAN)
	{
		new_values[CSS_TOP] = values[0];
		new_values[CSS_LEFT] = new_values[CSS_RIGHT] = values[1];
		new_values[CSS_BOTTOM] = values[2];
	}
	else
	{
		new_values[CSS_TOP] = values[0];
		new_values[CSS_RIGHT] = values[1];
		new_values[CSS_BOTTOM] = values[2];
		new_values[CSS_LEFT] = values[3];
		new_values[CSS_START] = values[4];
		new_values[CSS_END] = values[5];
	}
}
css_style_t css_parse(const char * style)
{
	css_style_t r;
	memset(&r, 0, sizeof(r));

	r.position[0] = NAN; r.position[1] = NAN; r.position[2] = NAN; r.position[3] = NAN;
	r.dimensions[0] = NAN; r.dimensions[1] = NAN;
	r.minDimensions[0] = NAN; r.minDimensions[1] = NAN;
	r.maxDimensions[0] = NAN; r.maxDimensions[1] = NAN;

	// named params
	typedef struct
	{
		const char * param;
		void * field; uint8_t size;
		struct
		{
			const char * key;
			int val;
		} values[6]; // max that we need
	} param_t;

	param_t params[] =
	{
		{
			"direction", &r.direction, sizeof(r.direction),
			{
				{"ltr",				CSS_DIRECTION_LTR},
				{"rtl",				CSS_DIRECTION_RTL},
				{NULL,				CSS_DIRECTION_INHERIT}
			}
		},
		{
			"flex-direction", &r.flex_direction, sizeof(r.flex_direction),
			{
				{"row",				CSS_FLEX_DIRECTION_ROW},
				{"row-reverse",		CSS_FLEX_DIRECTION_ROW_REVERSE},
				{"column",			CSS_FLEX_DIRECTION_COLUMN},
				{"column-reverse",	CSS_FLEX_DIRECTION_COLUMN_REVERSE},
				{NULL,				CSS_FLEX_DIRECTION_COLUMN}
			}
		},
		{
			"justify-content", &r.justify_content, sizeof(r.justify_content),
			{
				{"flex-start",		CSS_JUSTIFY_FLEX_START},
				{"flex-end",		CSS_JUSTIFY_FLEX_END},
				{"center",			CSS_JUSTIFY_CENTER},
				{"space-between",	CSS_JUSTIFY_SPACE_BETWEEN},
				{"space-around",	CSS_JUSTIFY_SPACE_AROUND},
				{NULL,				CSS_JUSTIFY_FLEX_START}
			}
		},
		{
			"align-content", &r.align_content, sizeof(r.align_content),
			{
				{"stretch",			CSS_ALIGN_STRETCH},
				{"center",			CSS_ALIGN_CENTER},
				{"flex-start",		CSS_ALIGN_FLEX_START},
				{"flex-end",		CSS_ALIGN_FLEX_END},
				{NULL,				CSS_ALIGN_STRETCH}
			}
		},
		{
			"align-items", &r.align_items, sizeof(r.align_items),
			{
				{"stretch",			CSS_ALIGN_STRETCH},
				{"center",			CSS_ALIGN_CENTER},
				{"flex-start",		CSS_ALIGN_FLEX_START},
				{"flex-end",		CSS_ALIGN_FLEX_END},
				{NULL,				CSS_ALIGN_STRETCH}
			}
		},
		{
			"align-self", &r.align_self, sizeof(r.align_self),
			{
				{"auto",			CSS_ALIGN_AUTO},
				{"stretch",			CSS_ALIGN_STRETCH},
				{"center",			CSS_ALIGN_CENTER},
				{"flex-start",		CSS_ALIGN_FLEX_START},
				{"flex-end",		CSS_ALIGN_FLEX_END},
				{NULL,				CSS_ALIGN_AUTO}
			}
		},
		{
			"position", &r.position_type, sizeof(r.position_type),
			{
				{"absolute",		CSS_POSITION_ABSOLUTE},
				{"relative",		CSS_POSITION_RELATIVE},
				{NULL,				CSS_POSITION_RELATIVE}
			}
		},
		{
			"flex-wrap", &r.flex_wrap, sizeof(r.flex_wrap),
			{
				{"nowrap",			CSS_NOWRAP},
				{"wrap",			CSS_WRAP},
				{NULL,				CSS_NOWRAP}
			}
		},
		{
			"overflow", &r.overflow, sizeof(r.overflow),
			{
				{"visible",			CSS_OVERFLOW_VISIBLE},
				{"hidden",			CSS_OVERFLOW_HIDDEN},
				{NULL,				CSS_OVERFLOW_VISIBLE}
			}
		}
	};
	size_t param_count = sizeof(params) / sizeof(param_t);

	char field[32] = {0};
	char value[6][32] = {0};

	size_t i = 0, j = 0, k = 0;
	while(style && style[i] != '\0')
	{
		memset(field, 0, sizeof(field));
		memset(value, 0, sizeof(value));

		i = css_skip_ws(style, i);

		while(!css_parse_is_fsep(style[i]))
			field[j++] = style[i++];
		j = 0;

		i = css_skip_ws(style, i);
		assert(style[i] == ':');
		++i;
		i = css_skip_ws(style, i);

		while(style[i] != ';' && style[i] != '\0')
		{
			while(!css_parse_is_vsep(style[i]))
				value[k][j++] = style[i++];
			k++;
			j = 0;
			i = css_skip_ws(style, i);
		}
		k = 0;
		assert(style[i] == ';');
		++i;

		bool set = false;
		for(size_t n = 0; n < param_count; ++n)
		{
			if(!strcmp(field, params[n].param))
				for(size_t m = 0; m < 6; ++m)
					if(!params[n].values[m].key || !strcmp(value[0], params[n].values[m].key))
					{
						switch(params[n].size) // this is needed to support bigend things
						{
						case 1: *(uint8_t*)params[n].field = params[n].values[m].val; break;
						case 2: *(uint16_t*)params[n].field = params[n].values[m].val; break;
						case 4: *(uint32_t*)params[n].field = params[n].values[m].val; break;
						case 8: *(uint64_t*)params[n].field = params[n].values[m].val; break;
						default: assert(0); break;
						}
						set = true;
						break;
					}
			if(set)
				break;
		}

		if(!set)
		{
			float num_value[6] = {0.0f};
			// we know that all following values are numbers
			for(size_t n = 0; n < 6; ++n)
			{
				if(strlen(value[n]))
					// TODO support px, %, etc, auto for NAN
					num_value[n] = atof(value[n]);
				else
					num_value[n] = NAN;
			}

			if(!strcmp(field, "flex"))				r.flex = num_value[0];
			else if(!strcmp(field, "margin"))		css_parse_shuffle6(num_value, r.margin);
			else if(!strcmp(field, "padding"))		css_parse_shuffle6(num_value, r.padding);
			else if(!strcmp(field, "border"))		css_parse_shuffle6(num_value, r.border);
			//else if(!strcmp(field, "border-radius"))r.border_radius = num_value[0]; // if you have it :)
			else if(!strcmp(field, "top"))			r.position[CSS_TOP] = num_value[0];
			else if(!strcmp(field, "bottom"))		r.position[CSS_BOTTOM] = num_value[0];
			else if(!strcmp(field, "left"))			r.position[CSS_LEFT] = num_value[0];
			else if(!strcmp(field, "right"))		r.position[CSS_RIGHT] = num_value[0];
			else if(!strcmp(field, "width"))		r.dimensions[CSS_WIDTH] = num_value[0];
			else if(!strcmp(field, "height"))		r.dimensions[CSS_HEIGHT] = num_value[0];
			else if(!strcmp(field, "max-width"))	r.maxDimensions[CSS_WIDTH] = num_value[0];
			else if(!strcmp(field, "max-height"))	r.maxDimensions[CSS_HEIGHT] = num_value[0];
			else if(!strcmp(field, "min-width"))	r.minDimensions[CSS_WIDTH] = num_value[0];
			else if(!strcmp(field, "min-height"))	r.minDimensions[CSS_HEIGHT] = num_value[0];
		}
	}

	return r;
}
