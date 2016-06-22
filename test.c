#include <css-micro-parser.h>

int main()
{
	css_style_t style = css_parse("right:20; align-self:center;");
	assert(style.position[CSS_RIGHT] == 20.0f);
	assert(style.align_self == CSS_ALIGN_CENTER);
	return 0;
}
