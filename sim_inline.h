static inline double normalize_angle(double t) {
    while (t >= 2 * M_PI)
	t -= 2 * M_PI;
    while (t < 0)
	t += 2 * M_PI;
    return t;
}

static inline double clip(double x, double low, double high) {
    return fmin(high, fmax(x, low));
}

static inline double clip_box(double x) {
    return clip(x, -BOX_DIM, BOX_DIM);
}

static inline double clip_speed(double x) {
    return clip(x, 0, MAX_SPEED);
}
