#!/usr/bin/env python3
import argparse, json, math, random

SUBJECTS = ["math","english","science","history","geography",
            "pe","art","music","computing","french"]


def gen(num_classes, num_subjects, periods, days, lessons_per_class,
        classes_per_teacher, unavail_prob, seed,
        double_prob=0.5, preferred_prob=0.1, undesired_prob=0.3):
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

    difficulties = {subj: rng.randint(1, 15) for subj in subjects}

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

    classes = {}
    for c in range(num_classes):
        grade = 6 + c // 6
        letter = "ABCDEFGH"[c % 6]
        name = f"{grade}{letter}"

        class_teachers = {subj: [teachers_by_subject[subj][c % len(teachers_by_subject[subj])]]
                          for subj in subjects}

        eligible = [s for s in subjects if class_load[s] >= 2]
        double_period = []
        if eligible and rng.random() < double_prob:
            n = rng.randint(1, min(2, len(eligible)))
            double_period = rng.sample(eligible, n)

        # preferred[day][period] -> subject or null (slot-level wish)
        weights = [class_load[s] for s in subjects]
        preferred = []
        for d in range(days):
            row = []
            for p in range(periods):
                if rng.random() < preferred_prob:
                    row.append(rng.choices(subjects, weights=weights, k=1)[0])
                else:
                    row.append(None)
            preferred.append(row)

        # undesired[day][period] -> list of subjects forbidden at that slot
        undesired = []
        for d in range(days):
            day_row = []
            for p in range(periods):
                if rng.random() < undesired_prob:
                    k = rng.randint(1, min(2, len(subjects)))
                    day_row.append(rng.sample(subjects, k))
                else:
                    day_row.append([])
            undesired.append(day_row)

        classes[name] = {
            "max_periods": periods,
            "days": days,
            "max_daily_difficulty": 67,
            "teachers": class_teachers,
            "course": dict(class_load),
            "double_period": double_period,
            "preferred": preferred,
            "undesired": undesired,
        }

    return {
        "difficulties": difficulties,
        "teachers": teachers,
        "classes": classes,
    }


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("classes", type=int)
    ap.add_argument("--subjects", type=int, default=10)
    ap.add_argument("--periods", type=int, default=6)
    ap.add_argument("--days", type=int, default=5)
    ap.add_argument("--lessons", type=int, default=28)
    ap.add_argument("--classes-per-teacher", type=int, default=5)
    ap.add_argument("--unavail-prob", type=float, default=0.3)
    ap.add_argument("--double-prob", type=float, default=0.5)
    ap.add_argument("--preferred-prob", type=float, default=0.1,
                    help="per-slot chance a preferred subject is set")
    ap.add_argument("--undesired-prob", type=float, default=0.1,
                    help="per-slot chance a subject is marked undesired")
    ap.add_argument("--seed", type=int, default=42)
    ap.add_argument("--out", default=None)
    a = ap.parse_args()

    total_slots = a.periods * a.days
    assert a.lessons <= total_slots, \
        f"lessons/class={a.lessons} > slots/class={total_slots}"

    data = gen(a.classes, a.subjects, a.periods, a.days, a.lessons,
               a.classes_per_teacher, a.unavail_prob, a.seed,
               a.double_prob, a.preferred_prob, a.undesired_prob)
    out = a.out or f"bench/{a.classes}.json"
    with open(out, "w") as f:
        json.dump(data, f, indent=2)

    print(f"Wrote {out}")
    print(f"  {a.classes} classes × {a.lessons} lessons = "
          f"{a.classes * a.lessons} lessons")
    print(f"  {len(data['teachers'])} teachers")
