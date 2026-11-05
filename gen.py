#!/usr/bin/env python3
import argparse, json, math

# Vibecoded DS benchmark because I have autism

SUBJECTS = ["math","english","science","history","geography",
            "pe","art","music","computing","french"]

def gen(num_classes, periods, days, lessons_per_class, classes_per_teacher):
    # Even distribution of lessons across subjects
    base = lessons_per_class // len(SUBJECTS)
    extra = lessons_per_class % len(SUBJECTS)
    class_load = {}
    for i, subj in enumerate(SUBJECTS):
        class_load[subj] = base + (1 if i < extra else 0)
        if class_load[subj] < 1:
            class_load[subj] = 1
    diff = lessons_per_class - sum(class_load.values())
    i = 0
    while diff != 0:
        s = SUBJECTS[i % len(SUBJECTS)]
        if diff > 0:
            class_load[s] += 1; diff -= 1
        elif class_load[s] > 1:
            class_load[s] -= 1; diff += 1
        i += 1

    # Pool of teachers per subject. Each teacher handles at most
    # 'classes_per_teacher' classes. Each class gets EXACTLY ONE teacher
    # per subject (single-element list).
    teachers_by_subject = {}
    all_teachers = []
    for subj in SUBJECTS:
        pool_size = max(1, math.ceil(num_classes / classes_per_teacher))
        pool = []
        for i in range(pool_size):
            name = f"{subj[:3].upper()}_{i}"
            pool.append(name)
            all_teachers.append(name)
        teachers_by_subject[subj] = pool

    classes = {}
    for c in range(num_classes):
        class_teachers = {}
        for subj in SUBJECTS:
            pool = teachers_by_subject[subj]
            class_teachers[subj] = [pool[c % len(pool)]]  # <-- single-element list
        classes[f"C{c:02d}"] = {
            "teachers": class_teachers,
            "course": dict(class_load)
        }

    return {
        "days": ["Mon","Tue","Wed","Thu","Fri"][:days],
        "periods_per_day": periods,
        "teachers": all_teachers,
        "subjects": SUBJECTS,
        "classes": classes
    }

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("classes", type=int)
    ap.add_argument("--periods", type=int, default=6)
    ap.add_argument("--days", type=int, default=5)
    ap.add_argument("--lessons", type=int, default=28,
                    help="lessons per class per week")
    ap.add_argument("--classes-per-teacher", type=int, default=5,
                    help="max classes assigned to any one teacher")
    ap.add_argument("--out", default=None)
    a = ap.parse_args()

    data = gen(a.classes, a.periods, a.days, a.lessons, a.classes_per_teacher)
    out = a.out or f"bench/{a.classes}.json"
    with open(out, "w") as f:
        json.dump(data, f, indent=2)

    total_slots = a.periods * a.days
    assert a.lessons <= total_slots, \
        f"lessons/class={a.lessons} > slots/class={total_slots}"
    n_per_subj = math.ceil(a.classes / a.classes_per_teacher)
    max_teacher_load = a.classes_per_teacher * math.ceil(a.lessons / len(SUBJECTS))
    print(f"Wrote {out}")
    print(f"  {a.classes} classes × {a.lessons} lessons = {a.classes * a.lessons} lessons total")
    print(f"  {len(data['teachers'])} teachers ({n_per_subj}/subject)")
    print(f"  each teacher: ≤{a.classes_per_teacher} classes, "
          f"≤~{max_teacher_load} lessons  (capacity: {total_slots})")
