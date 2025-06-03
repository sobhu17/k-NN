import random

n = 1000
d = 128
num_gen = d
index_name = "target_index"
field_name = "target_field"
mute = " > /dev/null"
data_type = "float"
min_value = 2
max_value = -2
space_type = "innerproduct"

def random_gen_byte():
  val = int(random.uniform(min_value, max_value))
  if val < -128:
    val = -128
  elif val > 127:
    val = 127
  return val

def random_gen_float():
  return random.uniform(min_value, max_value)

if data_type == "byte":
  gen_func = random_gen_byte
  data_type = "byte"
elif data_type == "binary":
  def binary_random_value():
    return random.randint(0, 128)
  gen_func = binary_random_value
  space_type = "hamming"
  d = 8
  num_gen = 1
else:
  gen_func = random_gen_float
  data_type = "float"


# Print a script

print(f"""
#!/bin/bash

# Create a mapping
curl -s -X PUT 'http://localhost:9200/{index_name}' -H 'Content-Type: application/json' -d '
{{
  "settings": {{
    "index": {{
      "knn": true,
      "use_compound_file": false,
      "knn.memory_optimized_search": false,
      "store": {{
        "type": "mmapfs"
      }}
    }}
  }},
  "mappings": {{
    "properties": {{
      "{field_name}": {{
        "type": "knn_vector",
        "dimension": {d},
        "data_type": "{data_type}",
        "space_type": "{space_type}",
        "method": {{
          "engine": "faiss",
          "name": "hnsw"
        }}
      }}
    }},
    "dynamic": false
  }}
}}
'

echo ""
echo "=============="
sleep 2

""")

k = 0
i = 0
while i < n:
  if i > 0 and (i % 10) == 0:
    lines = []
   
    for j in range(10):
      lines.append(f"{{ \"index\": {{ \"_index\": \"{index_name}\", \"_id\": \"{k}\" }} }}")
      k += 1
      v = []
      for j in range(num_gen):
        v.append(gen_func())
      price = gen_func()
      lines.append(f"{{ \"{field_name}\": {v} }}")

    print(f"""
curl -s -X POST 'http://localhost:9200/_bulk' -H 'Content-Type: application/json' -d '{"\n".join(lines)}
' {mute}""")
  i += 1

while k < n:
  lines = []
   
  for j in range(10):
    lines.append(f"{{ \"index\": {{ \"_index\": \"{index_name}\", \"_id\": \"{k}\" }} }}")
    k += 1
    v = []
    for j in range(num_gen):
      v.append(gen_func())
    price = gen_func()
    lines.append(f"{{ \"{field_name}\": {v} }}")
  print(f"""
curl -s -X POST 'http://localhost:9200/_bulk' -H 'Content-Type: application/json' -d '{"\n".join(lines)}
' {mute}""")



print(f"""
echo ""
echo "=============="
sleep 2

curl -s -X GET 'http://localhost:9200/{index_name}/_flush'
""")

print(f"""
curl -X POST "http://localhost:9200/{index_name}/_forcemerge?max_num_segments=1" -H "Content-Type: application/json"
""".strip())

v = []
for j in range(num_gen):
  v.append(gen_func())

print(f"""
curl -s -X GET 'http://localhost:9200/{index_name}/_search' -H 'Content-Type: application/json' -d '{{
  "size": 10,
  "query": {{
    "knn": {{
      "{field_name}": {{
        "vector": {v},
        "k": 10
      }}
    }}
  }}
}}' | jq . | head -n 20

echo ""
echo "\\n=============="
""")
