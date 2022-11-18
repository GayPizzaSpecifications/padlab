#include "stick.h"
#include "draw.h"

extern inline void InitDefaults(StickState* p);

vector RadialDeadzone(vector v, double min, double max)
{
	double mag = sqrt(v.x * v.x + v.y * v.y);

	if (mag <= min)
		return (vector){0.0, 0.0};

	if (mag >= max)
		return (vector){v.x / mag, v.y / mag};

	double rescale = (mag - min) / (max - min);
	return (vector){v.x / mag * rescale, v.y / mag * rescale};
}

point DigitalEight(vector v, double angle, double deadzone)
{
	const double absx = fabs(v.x);
	const double absy = fabs(v.y);
	point p = {0, 0};

	if (absx * angle >= absy)
	{
		if (absx > deadzone)
			p.x = signbit(v.x) ? -1 : 1;
	}
	else if (absy * angle > absx)
	{
		if (absy > deadzone)
			p.y = signbit(v.y) ? -1 : 1;
	}
	else if (absx + absy >= deadzone * (1.0 + angle))
	{
		p.x = signbit(v.x) ? -1 : 1;
		p.y = signbit(v.y) ? -1 : 1;
	}

	return p;
}

vector DigitalToVector(point p)
{
	const double dscale = (p.x && p.y) ? 1.0 / sqrt(2.0) : 1.0;
	return (vector){
		p.x ? copysign(dscale, (double)p.x) : 0.0,
		p.y ? copysign(dscale, (double)p.y) : 0.0};
}

static inline double AccelCurve(double x, double y)
{
	return (x * (x + y)) / (1.0 + y);
}

vector ApplyAcceleration(vector v, double y)
{
	double mag = sqrt(v.x * v.x + v.y * v.y);
	if (mag <= 0.0)
		return (vector){0.0, 0.0};

	double curve = AccelCurve(mag, y);
	return (vector){v.x / mag * curve, v.y / mag * curve};
}

extern inline void InitDefaults(StickState* p);

void DrawAnalogue(const rect* win, StickState* p)
{
	if (p->recalc)
	{
		p->compos = RadialDeadzone(p->rawpos, p->deadzone, 0.99);
		p->preaccel = sqrt(p->compos.x * p->compos.x + p->compos.y * p->compos.y);
		p->compos = ApplyAcceleration(p->compos, p->accelpow);
		p->postacel = sqrt(p->compos.x * p->compos.x + p->compos.y * p->compos.y);

		p->recalc = false;
	}

	double size = (double)(win->w > win->h ? win->h : win->w) * DISPLAY_SCALE;

	// range rect
	SetDrawColour(GREY3);
	const int rectSz = (int)round(size);
	DrawRect(
		win->x + (win->w - rectSz) / 2,
		win->y + (win->h - rectSz) / 2,
		rectSz, rectSz);

	const int ox = win->x + win->w / 2;
	const int oy = win->y + win->h / 2;

	// acceleration curve
	SetDrawColour(GREY5);
	const int accelsamp = (int)(sqrt(size) * 4.20);
	const double step = 1.0 / (double)accelsamp;
	double y1 = AccelCurve(0.0, p->accelpow);
	for (int i = 1; i <= accelsamp; ++i)
	{
		double y2 = AccelCurve(step * i, p->accelpow);
		DrawLine(
			win->x + (int)(step * (i - 1) * size) + (win->w - rectSz) / 2,
			win->y + (int)((1.0 - y1) * size) + (win->h - rectSz) / 2,
			win->x + (int)(step * i * size) + (win->w - rectSz) / 2,
			win->y + (int)((1.0 - y2) * size) + (win->h - rectSz) / 2);
		y1 = y2;
	}
	const int tickerx = (int)((p->preaccel - 0.5) * size);
	const int tickery = (int)((0.5 - p->postacel) * size);
	SetDrawColour(HILIGHT_PU1);
	DrawLine(
		ox + tickerx,
		win->y + (win->h - rectSz) / 2,
		ox + tickerx,
		win->y + (win->h + rectSz) / 2);
	SetDrawColour(HILIGHT_PU2);
	DrawLine(
		win->x + (win->w - rectSz) / 2,
		oy + tickery,
		win->x + (win->w + rectSz) / 2,
		oy + tickery);

	// guide circle
	SetDrawColour(GREY5);
	DrawCircle(ox, oy, rectSz / 2);

	SetDrawColour(GREY4);
	DrawCircle(ox, oy, (int)round(p->deadzone * size) / 2);

	// 0,0 line axis'
	SetDrawColour(GREY2);
	DrawLine(
		win->x, oy,
		win->x + win->w, oy);
	DrawLine(
		ox, win->y,
		ox, win->y + win->h);

	// compensated position
	SetDrawColour(HILIGHT_PU3);
	DrawCircleSteps(
		ox + (int)round(p->compos.x * size / 2.0),
		oy + (int)round(p->compos.y * size / 2.0),
		8, 16);
	DrawPoint(
		ox + (int)round(p->compos.x * size / 2.0),
		oy + (int)round(p->compos.y * size / 2.0));

	// raw position
	SetDrawColour(WHITE);
	DrawLine(
		ox + (int)round(p->rawpos.x * size / 2.0) - 4,
		oy + (int)round(p->rawpos.y * size / 2.0),
		ox + (int)round(p->rawpos.x * size / 2.0) + 4,
		oy + (int)round(p->rawpos.y * size / 2.0));
	DrawLine(
		ox + (int)round(p->rawpos.x * size / 2.0),
		oy + (int)round(p->rawpos.y * size / 2.0) - 4,
		ox + (int)round(p->rawpos.x * size / 2.0),
		oy + (int)round(p->rawpos.y * size / 2.0) + 4);
}

void DrawDigital(const rect* win, StickState* p)
{
	if (p->recalc)
	{
		p->digixy = DigitalEight(p->rawpos, p->digiangle, p->digideadzone);
		p->compos = DigitalToVector(p->digixy);
		p->recalc = false;
	}

	const double size = (double)(win->w > win->h ? win->h : win->w) * DISPLAY_SCALE;

	// range rect
	SetDrawColour(GREY3);
	const int rectSz = (int)round(size);
	DrawRect(
		win->x + (win->w - rectSz) / 2,
		win->y + (win->h - rectSz) / 2,
		rectSz, rectSz);

	// window centre
	const int ox = win->x + win->w / 2;
	const int oy = win->y + win->h / 2;

	// guide circle
	SetDrawColour(GREY5);
	int radius = rectSz / 2;
	DrawCircle(ox, oy, radius);

	// 0,0 line axis'
	SetDrawColour(GREY2);
	DrawLine(
		win->x, oy,
		win->x + win->w, oy);
	DrawLine(
		ox, win->y,
		ox, win->y + win->h);

	// calcuate points for the zone previews
	const double outerinvmag = 1.0 / sqrt(1.0 + p->digiangle * p->digiangle);
	const int outh = (int)round((size * outerinvmag) / 2.0);
	const int outq = (int)round((size * outerinvmag) / 2.0 * p->digiangle);
	const int innh = (int)round(p->digideadzone * size / 2.0);
	const int innq = (int)round(p->digideadzone * size / 2.0 * p->digiangle);

	SetDrawColour(GREY4);

	// angles preview
	DrawLine(ox - outq, oy - outh, ox - innq, oy - innh);
	DrawLine(ox + outq, oy - outh, ox + innq, oy - innh);
	DrawLine(ox + outh, oy - outq, ox + innh, oy - innq);
	DrawLine(ox + outh, oy + outq, ox + innh, oy + innq);
	DrawLine(ox + outq, oy + outh, ox + innq, oy + innh);
	DrawLine(ox - outq, oy + outh, ox - innq, oy + innh);
	DrawLine(ox - outh, oy + outq, ox - innh, oy + innq);
	DrawLine(ox - outh, oy - outq, ox - innh, oy - innq);

	// deadzone octagon
	DrawLine(ox - innq, oy - innh, ox + innq, oy - innh);
	DrawLine(ox + innq, oy - innh, ox + innh, oy - innq);
	DrawLine(ox + innh, oy - innq, ox + innh, oy - innq);
	DrawLine(ox + innh, oy - innq, ox + innh, oy + innq);
	DrawLine(ox + innh, oy + innq, ox + innq, oy + innh);
	DrawLine(ox + innq, oy + innh, ox - innq, oy + innh);
	DrawLine(ox - innq, oy + innh, ox - innh, oy + innq);
	DrawLine(ox - innh, oy + innq, ox - innh, oy - innq);
	DrawLine(ox - innh, oy - innq, ox - innq, oy - innh);

	// highlight active zone
	if (p->digixy.x || p->digixy.y)
	{
		const int x = p->digixy.x;
		const int y = p->digixy.y;

		SetDrawColour(HILIGHT_GR2);

		if (x)
		{
			if (y <= 0) DrawLine(ox + outh * x, oy - outq, ox + innh * x, oy - innq);
			if (!y) DrawLine(ox + innh * x, oy + innq, ox + innh * x, oy - innq);
			if (y >= 0) DrawLine(ox + outh * x, oy + outq, ox + innh * x, oy + innq);
		}

		if (y)
		{
			if (x <= 0) DrawLine(ox - outq, oy + outh * y, ox - innq, oy + innh * y);
			if (!x) DrawLine(ox + innq, oy + innh * y, ox - innq, oy + innh * y);
			if (x >= 0) DrawLine(ox + outq, oy + outh * y, ox + innq, oy + innh * y);
		}

		if (x && y)
		{
			DrawLine(ox + innh * x, oy + innq * y, ox + innq * x, oy + innh * y);
			DrawArc(ox, oy, radius,
				-(int)round(atan2(outerinvmag * p->digiangle * y, outerinvmag * x) * RAD2DEG),
				-(int)round(atan2(outerinvmag * y, outerinvmag * p->digiangle * x) * RAD2DEG));
		}
		else
		{
			const int hemi = (int)round(atan2(outerinvmag * p->digiangle, outerinvmag) * RAD2DEG);
			if (x > 0) DrawArc(ox, oy, radius, -hemi, hemi);
			else if (y < 0) DrawArc(ox, oy, radius, -hemi + 90, hemi + 90);
			else if (x < 0) DrawArc(ox, oy, radius, -hemi + 180, hemi + 180);
			else if (y > 0) DrawArc(ox, oy, radius, -hemi + 270, hemi + 270);
		}
	}

	// compensated position
	SetDrawColour(HILIGHT_GR3);
	DrawCircleSteps(
		ox + (int)round(p->compos.x * size / 2.0),
		oy + (int)round(p->compos.y * size / 2.0),
		8, 16);
	DrawPoint(
		ox + (int)round(p->compos.x * size / 2.0),
		oy + (int)round(p->compos.y * size / 2.0));

	// raw position
	SetDrawColour(WHITE);
	DrawLine(
		ox + (int)round(p->rawpos.x * size / 2.0) - 4,
		oy + (int)round(p->rawpos.y * size / 2.0),
		ox + (int)round(p->rawpos.x * size / 2.0) + 4,
		oy + (int)round(p->rawpos.y * size / 2.0));
	DrawLine(
		ox + (int)round(p->rawpos.x * size / 2.0),
		oy + (int)round(p->rawpos.y * size / 2.0) - 4,
		ox + (int)round(p->rawpos.x * size / 2.0),
		oy + (int)round(p->rawpos.y * size / 2.0) + 4);
}
