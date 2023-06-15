#include "joystick.h"

int joy_read_event(int fd, struct js_event* event) {
    ssize_t bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event)) return 0;

    // Failed to read event
    return -1;
}

size_t joy_get_axis_state(struct js_event* event, JoyAxisState axes[3]) {
    size_t axis = event->number / 2;

    if (axis < 3) {
        if (event->number % 2 == 0) {
            axes[axis].x = event->value;
        } else {
            axes[axis].y = event->value;
        }
    }

    return axis;
}
