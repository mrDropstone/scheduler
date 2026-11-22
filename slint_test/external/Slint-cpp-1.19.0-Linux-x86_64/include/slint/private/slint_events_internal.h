#pragma once

/* Generated with cbindgen:0.29.4 */

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include "private/slint_enums_internal.h"
#include "private/slint_point.h"
            #include "private/slint_builtin_structs_internal.h"
            namespace slint::cbindgen_private {
                struct Rect;
                using LogicalRect = Rect;
                using LogicalPoint = Point2D<float>;
                using LogicalLength = float;
            }

namespace slint {
namespace cbindgen_private {
namespace types {

/// Phase of a touch, gesture event or wheel event.
/// A touchpad is recognized as wheel event and therefore
/// we need to find out when the touch event starts and ends
enum class TouchPhase : uint8_t {
    /// The gesture began (e.g., first finger touched or platform gesture started).
    Started,
    /// The gesture is ongoing (e.g., fingers moved or platform gesture updated).
    Moved,
    /// The gesture completed normally.
    Ended,
    /// The gesture was cancelled (e.g., interrupted by the system) or the mouse wheel was used
    Cancelled,
};

/// The mouse events a backend can deliver to the runtime.
struct BackendMouseEvent {
    enum class Tag {
        /// The mouse or finger was pressed
        Pressed,
        /// The mouse or finger was released
        Released,
        /// The position of the pointer has changed.
        Moved,
        /// Wheel was operated.
        Wheel,
        /// A platform-recognized pinch gesture (macOS/iOS trackpad, Qt).
        PinchGesture,
        /// A platform-recognized rotation gesture (macOS/iOS trackpad, Qt).
        RotationGesture,
        /// The mouse exited the item or component
        Exit,
    };

    struct Pressed_Body {
        LogicalPoint position;
        PointerEventButton button;
        uint8_t click_count;
        int32_t touch_finger_id;

        bool operator==(const Pressed_Body& other) const {
            return position == other.position &&
                   button == other.button &&
                   click_count == other.click_count &&
                   touch_finger_id == other.touch_finger_id;
        }
        bool operator!=(const Pressed_Body& other) const {
            return position != other.position ||
                   button != other.button ||
                   click_count != other.click_count ||
                   touch_finger_id != other.touch_finger_id;
        }
    };

    struct Released_Body {
        LogicalPoint position;
        PointerEventButton button;
        uint8_t click_count;
        int32_t touch_finger_id;

        bool operator==(const Released_Body& other) const {
            return position == other.position &&
                   button == other.button &&
                   click_count == other.click_count &&
                   touch_finger_id == other.touch_finger_id;
        }
        bool operator!=(const Released_Body& other) const {
            return position != other.position ||
                   button != other.button ||
                   click_count != other.click_count ||
                   touch_finger_id != other.touch_finger_id;
        }
    };

    struct Moved_Body {
        LogicalPoint position;
        int32_t touch_finger_id;

        bool operator==(const Moved_Body& other) const {
            return position == other.position &&
                   touch_finger_id == other.touch_finger_id;
        }
        bool operator!=(const Moved_Body& other) const {
            return position != other.position ||
                   touch_finger_id != other.touch_finger_id;
        }
    };

    struct Wheel_Body {
        LogicalPoint position;
        float delta_x;
        float delta_y;
        TouchPhase phase;

        bool operator==(const Wheel_Body& other) const {
            return position == other.position &&
                   delta_x == other.delta_x &&
                   delta_y == other.delta_y &&
                   phase == other.phase;
        }
        bool operator!=(const Wheel_Body& other) const {
            return position != other.position ||
                   delta_x != other.delta_x ||
                   delta_y != other.delta_y ||
                   phase != other.phase;
        }
    };

    struct PinchGesture_Body {
        LogicalPoint position;
        float delta;
        TouchPhase phase;

        bool operator==(const PinchGesture_Body& other) const {
            return position == other.position &&
                   delta == other.delta &&
                   phase == other.phase;
        }
        bool operator!=(const PinchGesture_Body& other) const {
            return position != other.position ||
                   delta != other.delta ||
                   phase != other.phase;
        }
    };

    struct RotationGesture_Body {
        LogicalPoint position;
        float delta;
        TouchPhase phase;

        bool operator==(const RotationGesture_Body& other) const {
            return position == other.position &&
                   delta == other.delta &&
                   phase == other.phase;
        }
        bool operator!=(const RotationGesture_Body& other) const {
            return position != other.position ||
                   delta != other.delta ||
                   phase != other.phase;
        }
    };

    Tag tag;
    union {
        Pressed_Body pressed;
        Released_Body released;
        Moved_Body moved;
        Wheel_Body wheel;
        PinchGesture_Body pinch_gesture;
        RotationGesture_Body rotation_gesture;
    };

    static BackendMouseEvent Pressed(const LogicalPoint &position,
                                     const PointerEventButton &button,
                                     const uint8_t &click_count,
                                     const int32_t &touch_finger_id) {
        BackendMouseEvent result;
        ::new (&result.pressed.position) (LogicalPoint)(position);
        ::new (&result.pressed.button) (PointerEventButton)(button);
        ::new (&result.pressed.click_count) (uint8_t)(click_count);
        ::new (&result.pressed.touch_finger_id) (int32_t)(touch_finger_id);
        result.tag = Tag::Pressed;
        return result;
    }

    bool IsPressed() const {
        return tag == Tag::Pressed;
    }

    static BackendMouseEvent Released(const LogicalPoint &position,
                                      const PointerEventButton &button,
                                      const uint8_t &click_count,
                                      const int32_t &touch_finger_id) {
        BackendMouseEvent result;
        ::new (&result.released.position) (LogicalPoint)(position);
        ::new (&result.released.button) (PointerEventButton)(button);
        ::new (&result.released.click_count) (uint8_t)(click_count);
        ::new (&result.released.touch_finger_id) (int32_t)(touch_finger_id);
        result.tag = Tag::Released;
        return result;
    }

    bool IsReleased() const {
        return tag == Tag::Released;
    }

    static BackendMouseEvent Moved(const LogicalPoint &position,
                                   const int32_t &touch_finger_id) {
        BackendMouseEvent result;
        ::new (&result.moved.position) (LogicalPoint)(position);
        ::new (&result.moved.touch_finger_id) (int32_t)(touch_finger_id);
        result.tag = Tag::Moved;
        return result;
    }

    bool IsMoved() const {
        return tag == Tag::Moved;
    }

    static BackendMouseEvent Wheel(const LogicalPoint &position,
                                   const float &delta_x,
                                   const float &delta_y,
                                   const TouchPhase &phase) {
        BackendMouseEvent result;
        ::new (&result.wheel.position) (LogicalPoint)(position);
        ::new (&result.wheel.delta_x) (float)(delta_x);
        ::new (&result.wheel.delta_y) (float)(delta_y);
        ::new (&result.wheel.phase) (TouchPhase)(phase);
        result.tag = Tag::Wheel;
        return result;
    }

    bool IsWheel() const {
        return tag == Tag::Wheel;
    }

    static BackendMouseEvent PinchGesture(const LogicalPoint &position,
                                          const float &delta,
                                          const TouchPhase &phase) {
        BackendMouseEvent result;
        ::new (&result.pinch_gesture.position) (LogicalPoint)(position);
        ::new (&result.pinch_gesture.delta) (float)(delta);
        ::new (&result.pinch_gesture.phase) (TouchPhase)(phase);
        result.tag = Tag::PinchGesture;
        return result;
    }

    bool IsPinchGesture() const {
        return tag == Tag::PinchGesture;
    }

    static BackendMouseEvent RotationGesture(const LogicalPoint &position,
                                             const float &delta,
                                             const TouchPhase &phase) {
        BackendMouseEvent result;
        ::new (&result.rotation_gesture.position) (LogicalPoint)(position);
        ::new (&result.rotation_gesture.delta) (float)(delta);
        ::new (&result.rotation_gesture.phase) (TouchPhase)(phase);
        result.tag = Tag::RotationGesture;
        return result;
    }

    bool IsRotationGesture() const {
        return tag == Tag::RotationGesture;
    }

    static BackendMouseEvent Exit() {
        BackendMouseEvent result;
        result.tag = Tag::Exit;
        return result;
    }

    bool IsExit() const {
        return tag == Tag::Exit;
    }

    bool operator==(const BackendMouseEvent& other) const {
        if (tag != other.tag) {
            return false;
        }
        switch (tag) {
            case Tag::Pressed: return pressed == other.pressed;
            case Tag::Released: return released == other.released;
            case Tag::Moved: return moved == other.moved;
            case Tag::Wheel: return wheel == other.wheel;
            case Tag::PinchGesture: return pinch_gesture == other.pinch_gesture;
            case Tag::RotationGesture: return rotation_gesture == other.rotation_gesture;
            default: break;
        }
        return true;
    }

    bool operator!=(const BackendMouseEvent& other) const {
        return !(*this == other);
    }

    private:
    BackendMouseEvent() {

    }
    public:


    ~BackendMouseEvent() {
        switch (tag) {
            case Tag::Pressed: pressed.~Pressed_Body(); break;
            case Tag::Released: released.~Released_Body(); break;
            case Tag::Moved: moved.~Moved_Body(); break;
            case Tag::Wheel: wheel.~Wheel_Body(); break;
            case Tag::PinchGesture: pinch_gesture.~PinchGesture_Body(); break;
            case Tag::RotationGesture: rotation_gesture.~RotationGesture_Body(); break;
            default: break;
        }
    }

    BackendMouseEvent(const BackendMouseEvent& other)
     : tag(other.tag) {
        switch (tag) {
            case Tag::Pressed: ::new (&pressed) (Pressed_Body)(other.pressed); break;
            case Tag::Released: ::new (&released) (Released_Body)(other.released); break;
            case Tag::Moved: ::new (&moved) (Moved_Body)(other.moved); break;
            case Tag::Wheel: ::new (&wheel) (Wheel_Body)(other.wheel); break;
            case Tag::PinchGesture: ::new (&pinch_gesture) (PinchGesture_Body)(other.pinch_gesture); break;
            case Tag::RotationGesture: ::new (&rotation_gesture) (RotationGesture_Body)(other.rotation_gesture); break;
            default: break;
        }
    }
    BackendMouseEvent& operator=(const BackendMouseEvent& other) {
        if (this != &other) {
            this->~BackendMouseEvent();
            new (this) BackendMouseEvent(other);
        }
        return *this;
    }
};

}  // namespace types
}  // namespace cbindgen_private
}  // namespace slint
