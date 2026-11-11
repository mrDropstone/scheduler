#!/usr/bin/env python3
import argparse, json, math, random

SUBJECTS = ["math","english","science","history","geography",
            "pe","art","music","computing","french"]

def gen(num_classes, num_subjects, periods, days, lessons_per_class,
        classes_per_teacher, unavail_prob, seed):
    rng = random.Random(seed)
    subjects = SUBJECTS[:num_subjects]

    # --- distribute lessons across subjects ---
    base = lessons_per_class // len(subjects)
    extra = lessons_per_class % len(subjects)
    class_load = {}
    for i, subj in enumerate(subjects):
        class_load[subj] = base + (1 if i < extra else 0)
        if class_load[subj] < 1:
            class_load[subj] = 1
    diff = lessons_per_class - sum(class_load.values())
    i = 0
    while diff != 0:
        s = subjects[i % len(subjects)]
        if diff > 0:
            class_load[s] += 1; diff -= 1
        elif class_load[s] > 1:
            class_load[s] -= 1; diff += 1
        i += 1

    # --- teacher pool per subject ---
    teachers_by_subject = {}
    all_teachers = []
    for subj in subjects:
        pool_size = max(1, math.ceil(num_classes / classes_per_teacher))
        pool = []
        for j in range(pool_size):
            name = f"{subj[:3].upper()}_{j}"
            pool.append(name)
            all_teachers.append(name)
        teachers_by_subject[subj] = pool

    # --- difficulty per subject (freeform, tune to taste) ---
    difficulties = {subj: rng.randint(1, 15) for subj in subjects}

    # --- teacher objects with per-day unavailability ---
    # unavailability[d] = list of period indices where this teacher cannot teach on day d
    teachers = {}
    for t in all_teachers:
        unavailability = []
        for d in range(days):
            if rng.random() < unavail_prob:
                n_blocked = rng.randint(1, max(1, min(2, periods - 1)))
                blocked = sorted(rng.sample(range(periods), n_blocked))
            else:
                blocked = []
            unavailability.append(blocked)
        teachers[t] = {"unavailability": unavailability}

    # --- classes ---
    classes = {}
    for c in range(num_classes):
        grade = 6 + c // 6
        letter = "ABCDEFGH"[c % 6]
        name = f"{grade}{letter}"
        class_teachers = {}
        for subj in subjects:
            pool = teachers_by_subject[subj]
            class_teachers[subj] = [pool[c % len(pool)]]   # one teacher per subject
        classes[name] = {
            "teachers" : class_teachers,
            "course" : dict(class_load),
            "max_periods" : periods,
            "days" : days
        }

    return {
        "days": ["Mon","Tue","Wed","Thu","Fri"][:days],
        "subjects": subjects,
        "difficulties": difficulties,
        "teachers": teachers,
        "classes": classes,
    }

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("classes", type=int)
    ap.add_argument("--subjects", type=int, default=10,
                    help="how many of the default subjects to use (2..10)")
    ap.add_argument("--periods", type=int, default=6)
    ap.add_argument("--days", type=int, default=5)
    ap.add_argument("--lessons", type=int, default=28,
                    help="lessons per class per week")
    ap.add_argument("--classes-per-teacher", type=int, default=5)
    ap.add_argument("--unavail-prob", type=float, default=0.3,
                    help="chance a teacher is unavailable on a given day")
    ap.add_argument("--seed", type=int, default=42)
    ap.add_argument("--out", default=None)
    a = ap.parse_args()

    total_slots = a.periods * a.days
    assert a.lessons <= total_slots, \
        f"lessons/class={a.lessons} > slots/class={total_slots}"

    data = gen(a.classes, a.subjects, a.periods, a.days, a.lessons,
               a.classes_per_teacher, a.unavail_prob, a.seed)
    out = a.out or f"bench/{a.classes}.json"
    with open(out, "w") as f:
        json.dump(data, f, indent=2)

    n_per_subj = math.ceil(a.classes / a.classes_per_teacher)
    max_load = a.classes_per_teacher * math.ceil(a.lessons / a.subjects)
    print(f"Wrote {out}")
    print(f"  {a.classes} classes × {a.lessons} lessons = "
          f"{a.classes * a.lessons} lessons")
    print(f"  {len(data['teachers'])} teachers "
          f"({n_per_subj}/subject), each ≤~{max_load} lessons, "
          f"capacity {total_slots}")
