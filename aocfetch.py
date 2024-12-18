import sys
import os
import shutil
import time
import urllib.request
import ssl

SESSION_COOKIE_PATH = "session.txt"
SRC_DIRECTORY_PATH = os.path.dirname(os.path.abspath(__file__))
USAGE_STRING = f'Usage: aocfetch N (to prepare day N; make sure to put your session cookie into "{SESSION_COOKIE_PATH}")'

def download_input(day_n: int, session: str):
    request = urllib.request.Request(f'https://adventofcode.com/2024/day/{day_n}/input')
    request.add_header("Cookie", f'session={session}')
    context = ssl._create_unverified_context()
    try: 
        with urllib.request.urlopen(request, context=context) as response: 
            return response.read().decode("utf-8")
    except:
        return None
    
def read_session_file(session_path):
    with open(session_path, "r") as f: 
        for line in f:
            line = line.lstrip().rstrip()
            if len(line) > 0: 
                return line
    return None

if __name__ == "__main__":
    if len(sys.argv) <= 1: 
        print(USAGE_STRING)
        sys.exit(0)

    os.chdir(SRC_DIRECTORY_PATH)

    try: 
        day_n = int(sys.argv[1])
    except ValueError: 
        print("Error: Invalid day argument (must be between 1 and 25).")
        print(USAGE_STRING)
        sys.exit(-1)

    if len(str(day_n)) == 1:
        day_str = f'day-0{day_n}'
    else:
        day_str = f'day-{day_n}'

    if os.path.exists(os.path.join("input", f'{day_str}.txt')):
        print(f'Day {day_n} is already ready to solve. Done.')
        sys.exit(0)

    session_cookie = read_session_file(SESSION_COOKIE_PATH)
    if not session_cookie: 
        print(f'Error: No session cookie in file {SESSION_COOKIE_PATH}.')
        print(USAGE_STRING)
        sys.exit(-1)

    print("Downloading input...")
    time.sleep(0.2)
    input_txt = download_input(day_n, session_cookie)
    if not input_txt: 
        print('Error: Could not fetch input file. Make sure your session cookie is valid.\nProceeding to create empty example-input file and to copy code template...')

    print("Creating input files...")
    if input_txt:
        with open(os.path.join("input", f'{day_str}.txt'), "w") as input_file:
            input_file.write(input_txt)

    with open(os.path.join("input", f'{day_str}-example.txt'), "w") as example_input_file:
        example_input_file.write("")
    
    print("Copying code template...")
    if not os.path.exists(day_str):
        os.mkdir(day_str)
    code_outfile = f'{day_str}/{day_str}.cpp'
    if not os.path.exists(code_outfile):
        shutil.copyfile('day-xy/day-xy.cpp', code_outfile)
    
    print(f'Done. Have fun solving day {day_n} :)')
