import json
import os
import random

os.makedirs("data", exist_ok=True)

# 1. small.json (~150 B)
small = {
    "id": 1,
    "name": "Thoth Benchmark",
    "active": True,
    "score": 99.9,
    "tags": ["c++", "json", "fast"],
    "address": {"street": "Main St", "city": "Tech City"}
}
with open("data/small.json", "w") as f: json.dump(small, f)

# 2. medium.json (~14 KB)
medium = {"total": 40, "page": 1, "users": []}
for i in range(40):
    medium["users"].append({
        "id": i,
        "name": f"User {i}",
        "email": f"user{i}@example.com",
        "address": {"street": f"{i} Byte Way", "city": "RAMville", "zip": f"100{i:02d}"},
        "bio": "Lorem ipsum dolor sit amet, consectetur adipiscing elit. " * 2
    })
with open("data/medium.json", "w") as f: json.dump(medium, f)

# 3. large.json (~5 MB)
large = {"data": []}
for i in range(10000):
    large["data"].append({
        "id": i,
        "name": f"Item {i}",
        "value": random.random(),
        "description": "Nn sei como vou me assumir pra minha familia ainda. Ok, eu preciso arrumar fonte de renda estavel antes de tentar qualquer loucura, mas e dps? E se nn me aceitarem? Pra onde eu vou nas ferias? Com quem eu vou conversar sobre o dia? Quem vai perguntar sobre o meu dia? E os outros parentes?... Minha mae e muito religiosa. Mesmo sem muita condicao eles sempre me apoiaram nos estudos entao eu quero muito retribuir td oq eles fizeram por mim (incluindo financeiramente), mas isso me causa um pouco de apreensao...",
        "flags": [True, False, True]
    })
with open("data/large.json", "w") as f: json.dump(large, f)

# 4. nested.json (~15 KB)
# Goes 10 levels searching "child" and then read "meta" -> "tag"
nested = current = {}
for i in range(20):
    current["level"] = 20 - i
    current["meta"] = {"tag": f"alvo_nivel_{i}"} # Garante que a tag exista
    current["child"] = {}
    current = current["child"]
with open("data/nested.json", "w") as f: json.dump(nested, f)

# 5. numbers.json (~5 KB) - Matrix 20x20
numbers = [[random.uniform(-1000.0, 1000.0) for _ in range(20)] for _ in range(20)]
with open("data/numbers.json", "w") as f: json.dump(numbers, f)

# 6. array.json (~15 KB) - 200 elements
arr = []
for i in range(200):
    arr.append({
        "x": random.random() * 100,
        "y": random.random() * -100,
        "label": f"label_{i}",
        "enabled": i % 2 == 0
    })
with open("data/array.json", "w") as f: json.dump(arr, f)

# 7. strings.json (~4 KB) - Unicode and escaping
strings = {
    f"str_{i}": f"line 1\nline 2\t\"Escape\" \u2728 emoji \u6f22\u5b57 {i}"
    for i in range(50)
}

with open("data/strings.json", "w") as f: json.dump(strings, f)

# 8. raw_strings.json (~4 KB) - just ASCII
raw_strings = {
    f"str_{i}": f"Thoth uses COW strings, so it's faster to use when no Unicode/escaping if found"
    for i in range(50)
}
with open("data/raw_strings.json", "w") as f: json.dump(raw_strings, f)
