# Game Server Study

게임 서버 개발 스터디 예제 및 실습 코드를 작성하고 관리하는 저장소입니다.

---

## 🛠️ 컴파일 및 실행 방법

각 예제는 단일 C 파일 또는 여러 C 파일로 구성되어 있으며, 터미널(GCC), VS Code 빌드 작업, 또는 `Makefile`을 통해 컴파일하고 실행할 수 있습니다.

### 1. 터미널 (GCC 직접 컴파일)

예제 파일이 위치한 디렉토리나 프로젝트 루트에서 아래와 같이 컴파일 후 실행합니다:

```bash
# 컴파일: gcc -o <출력파일명> <소스파일.c>
gcc -g -o ch01/marshal ch01/marshal.c

# 실행
./ch01/marshal
```

### 2. VS Code에서 빌드

VS Code의 기본 빌드 태스크가 설정되어 있습니다.

1. 빌드하려는 C 소스 파일(`.c`)을 에디터에서 엽니다.
2. 단축키 `Ctrl + Shift + B` (macOS: `Cmd + Shift + B`)를 눌러 **"C/C++: gcc 활성 파일 빌드"**를 실행합니다.
3. 생성된 실행 파일은 터미널에서 `./<실행파일명>`으로 실행합니다.

### 3. Makefile을 이용한 다중 파일 빌드 (예: `ch02`)

여러 개의 `.c` 파일을 함께 컴파일하거나 외부 라이브러리(`-lpq` 등)를 링크해야 하는 경우 `Makefile`을 사용합니다.

#### 빌드 및 실행 방법

`Makefile`이 위치한 디렉토리에서 `make` 명령어를 실행합니다.

```bash
# 디렉토리 이동 후 빌드 및 실행
cd ch02
make
./svr

# 또는 프로젝트 루트에서 -C 옵션으로 빌드 후 실행
make -C ch02
./ch02/svr
```

#### Makefile 작성 방법 (`ch02/Makefile` 기준)

```makefile
CC = gcc
CFLAGS = -g -lpq
SRCS = svr.c session.c
TARGET = svr

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)
```

- **`CC`**: 사용할 컴파일러를 지정합니다. (예: `gcc`)
- **`CFLAGS`**: 컴파일 및 링크 옵션을 지정합니다. (예: `-g` 디버그 심볼 포함, `-lpq` PostgreSQL 라이브러리 링크)
- **`SRCS`**: 함께 컴파일할 소스 파일 목록을 공백으로 구분해 나열합니다.
- **`TARGET`**: 생성할 최종 실행 파일 이름입니다.
- **규칙 (`타겟: 의존성`)**: `$(SRCS)` 파일 중 하나라도 수정되면 아래 명령어를 실행해 `$(TARGET)`을 다시 빌드합니다.
- **주의사항**: 타겟 아래의 실행 명령어 줄 앞에는 반드시 공백(Space)이 아닌 **탭(Tab)** 문자를 사용해야 합니다.

---

## 🐘 데이터베이스 (PostgreSQL)

DB 연동 예제는 **PostgreSQL**을 사용하며, 프로젝트 루트의 `docker-compose.yml`을 통해 컨테이너로 실행할 수 있습니다.

### 1. 실행 및 종료

```bash
# PostgreSQL 컨테이너 백그라운드 실행
docker compose up -d

# 컨테이너 중지 및 제거 (데이터는 ./data 디렉토리에 유지됨)
docker compose down
```

### 2. 접속 정보

| 항목 | 값 |
| :--- | :--- |
| **Host** | `localhost` |
| **Port** | `5432` |
| **User** | `postgres` |
| **Password** | `postgres` |
| **Database** | `mydb` |

### 3. 컨테이너 내부 `psql` 접속

```bash
docker compose exec db psql -U postgres -d mydb
```

---

> **참고**: 컴파일된 바이너리 및 실행 파일(`.out`, `.exe` 등)은 Git 추적에서 제외(`.gitignore`)됩니다.
