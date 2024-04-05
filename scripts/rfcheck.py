from pathlib import Path

files = list(Path('.').glob('**/*.ino'))

for file in files:
    with file.open() as f:
        for line_num, line in enumerate(f):
            if "rf95.send(" in line.strip():
                start_quote = line.find('"')
                end_quote = line.rfind('"')
                if start_quote == -1 and end_quote == -1:
                    continue
                length = end_quote - start_quote
                # print(line[start_quote+1:end_quote])
                print(f"{file}:{line_num+1} = {length}")


