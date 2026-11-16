# Game Server Study

게임 서버 개발 스터디 예제 및 실습 코드를 작성하고 관리하는 저장소입니다.

---

## 🛠️ 컴파일 및 실행 방법

각 예제는 단일 C 파일 형태로 제공되며, 터미널(GCC) 또는 VS Code 빌드 작업을 통해 컴파일하고 실행할 수 있습니다.

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
