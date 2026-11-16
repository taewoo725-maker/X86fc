; 만든이: KimTaeWoo
; X86FC - Copyright (C) 2026 Kim Taewoo. SPDX-License-Identifier: GPL-3.0-or-later
; ======================================================================
; X86FC - x86 File Commander, MSX-DOS2 / Nextor 파일 관리자 (프로그램 이름 X86FC.COM)
;
; 버전 이력 (최신이 위)
; 0.12.35: Shift 를 누르고 있는 동안 기능키 줄이 F6~F10 으로 유지되게 (SNSMAT + NEWKEY 이중 확인, 뗌은 3 jiffy 이상 계속일 때만). 패널 항목 한도 MAXE 192 -> 190
; 0.12.34: 빌드 도구만 변경 — mkovl.py 가 NOHGL(글꼴 없는 시험용) OVL 을 실행파일 폴더에 쓰지 않고, rel_new.sh 가 릴리스 OVL 크기를 검사 (COM·OVL 은 0.12.33 과 같음)
; 0.12.33: SCREEN 7·7H 팝업을 닫을 때 화면 전체가 아니라 팝업이 덮었던 줄만 다시 그림
; 0.12.32: 팝업 오버레이(드라이브·열기 목록·사용자 메뉴·정렬) 안의 draw_all 이 균일 격자로 28줄 표를 그려 화면 밖의 DOS 화면 맡김을 깨뜨리던 것 고침 (0.12.21 부터)
; 0.12.31: 선 띠·줄 높이 재설계 — 선과 글자 사이 보이는 간격 1점, 7H 목록 20줄(tight 20줄)
; 0.12.30: F4 로 글자 화면 0 에서 SCREEN 7 로 바꾸면 줄 표(얇은 선 배치·틀·맨 위 선)가 적용되지 않던 것 고침
; 0.12.29: SCREEN 7·7H 맨 위 가로 테두리선(1점)으로 틀을 닫음 (선 띠를 3점으로 줄여 목록 줄 수 유지)
; 0.12.28: SCREEN 7·7H 좌우 바깥 여백에 1점 세로 테두리선(x 14, 491)을 넣고 가로선 3개를 테두리까지 이어 패널 전체를 틀로 감쌈
; 0.12.27: X86FC.INI 연결 줄의 내부 명령 #VIEW(내부 보기)·#EDIT(내부 편집기). 예: HLP = #VIEW, HLP = #VIEW;AKID %s (풀다운)
; 0.12.26: 활성 패널을 바꿀 때 정보 줄의 이전 화살표 아이콘이 지워지지 않던 것 고침 (SCREEN 7·7H: 줄 해시 무효화)
; 0.12.25: 화면 표시선(패널 가로선·세로선·팝업 테두리)을 1점 가는 선으로, 교차부·모서리를 틈 없이 이어 붙임
; 0.12.24: 소스 주석의 깨진 글자를 한글로 복구하고 버전 이력을 판마다 한 줄로 정리 (COM·OVL 은 0.12.23 과 같음)
; 0.12.23: F5 BASIC 에서 CALL SYSTEM 으로 돌아올 때 CALL SYSTEM("경로") 로 바꿔 실행(Nextor 실기에서 키 버퍼 주입이 안 되던 것 대비), 안 되면 예전 키 버퍼 방식
; 0.12.22: 정보 줄 화살표 위치 맞춤
; 0.12.21: SCREEN 7·7H 패널 구분선을 얇은 선으로
; 0.12.20: SCREEN 7·7H 아이콘 모양 정비
; 0.12.17~0.12.19: 7H 편집기(F3)에서 한글 입력을 3단계로 완성
; 0.12.16: SCREEN 7 26줄 옵션, 7H 9점 줄 옵션
; 0.12.15: 메모리(BSS) 정리
; 0.12.14: INI 통합 — SET·MNU·HIS·QD·HGL 을 X86FC.INI·X86FC.OVL 로 합쳐 COM+OVL+INI 세 파일로, INI 크기 제한 없앰
; 0.12.13: 정보 줄 화살표 위치, F4 이름 4Graph → 4Screen
; 0.12.12: 팝업 창의 한글 표시(7H)
; 0.12.11: 7H 에서 한글 이름 입력(이름 바꾸기·폴더 만들기, 긴 이름으로 저장)
; 0.12.10: Quick-Dir·사용자 메뉴를 ↑↓ + RETURN 으로 고르기
; 0.12.9: 7H 스크롤 속도 개선(VDP 복사)
; 0.12.8: 이후 작업의 기준 판
; 0.12.7 (풀다운 깜빡임 개선: ↑↓ 로 줄을 고르는 창(열기 목록 vl_manage, 파일 선택 pick_file, F8 정렬, 찾기 결과, 트리)이 키마다 화면·창 전체를 다시 그리지 않고 바뀐 두 줄만 다시 그림(목록이 넘어갈 때만 목록 줄들). 열기 목록 ESC 뒤 팝업 자국 지움, 파일 선택 창 맨 아래 테두리에 항목 글이 겹치던 것 고침)
; 0.12.6 (프로그램 실행 방식 변경: 적재 코드(D000h, _FORK)를 없애고, 상태를 저장한 뒤 X86FC 를 완전히 끝내고 키 버퍼로 고정 배치 파일(X86FCRUN.BAT)을 실행시켜 COMMAND2 가 프로그램을 돌린 뒤 X86FC 를 다시 띄움 — TPA 에 X86FC 코드가 없으니 프로그램이 메모리를 얼마나 덮어써도 안전)
; 0.12.4 (F2 보기: 7H 에서 한글(UTF-8·EUC-KR) 보기, 보기를 오버레이 4 로 옮김)
; 0.12.3 (사용자 메뉴(Ctrl+G)는 X86FC.MNU 에 저장(INI 가 커도 상관없음, 예전 INI 의 MENU 줄은 처음 한 번 MNU 로 옮김))
; 0.12.2 (F4·F8 로 고른 화면 방식·정렬은 X86FC.SET 에 저장(2000 바이트 넘는 INI 에서 저장·읽기가 안 되던 것 고침))
; 0.12.1 (파일 목록은 다시 8.3 이름, 긴 이름은 아래 정보 줄에만)
; 0.12.0 (SCREEN 7 한글 모드 7H: F4 = 글자 -> SCREEN 7 -> 7H 순환, 한글 이름을 Galmuri9 9x9 점글꼴(X86FC.HGL)로, 줄 높이 10점·목록 15줄)
; 0.11.10 (정보 줄 화살표를 굵은 블록 화살표로)
; 0.11.9 (SCREEN 7 아이콘을 8x8 로(옆 빈 칸으로 2점 넘침), 정보 줄 화살표도 8x8 한 칸)
; 0.11.8 (정보 줄 표시를 <- -> (SCREEN 7 은 화살표 아이콘)로 하고 한 칸 띄고 긴 이름)
; 0.11.7 (정보 줄 맨 앞에 선택된 창 표시(< 왼쪽, > 오른쪽, SCREEN 7 은 삼각형 아이콘))
; 0.11.6 (아래 정보 줄을 한 줄로: 활성 패널의 고른 파일 긴 이름 + 뒤에 N items, 목록 18줄)
; 0.11.5 (위·아래 한 칸은 커서 줄 두 개만 다시 그림, 끝낼 때 글자 화면을 다 지움)
; 0.11.4 (32MB 넘는 볼륨(Nextor 큰 하드)에서도 긴 이름을 읽음: BPB 의 32 비트 전체 섹터 수와 _RDDRV/_WRDRV 로 32 비트 섹터 번호 처리)
; 0.11.3 (F4 로 SCREEN 7 을 고르면 INI 에 SCREEN = 1 로 잘못 저장돼 다음 시작·프로그램 다녀온 뒤 SCREEN 0 이 되던 것 고침. 기능키 줄 위의 가로선이 패널보다 한 칸 길던 것 고침)
; 0.11.2 (INI 를 2000 바이트까지 읽음: 1023 넘는 INI 에서 SCREEN 줄이 무시돼 프로그램 다녀오면 SCREEN 0 으로 돌아가던 것 고침)
; 0.11.1: 도움말 정리
; 0.11.0 (**오버레이**: 찾기·트리·보기·편집기·사용자 메뉴를 X86FC.OVL 로 떼어 본체 7KB 줄임, 폴더당 항목 216개)
; 0.10.5: F4 로 바꾼 화면 방식을 INI 의 SCREEN 줄에 저장
; 0.10.4: 연결 없는 확장자는 "연결할까요?" 팝업으로 물어보고 예면 프로그램 목록 팝업
; 0.10.3: 연결 없는 확장자를 실행하면 프로그램 목록 팝업
; 0.10.2: 목록에 긴 이름을 보임
; 0.10.1: INI 의 SORT(F8 로 고르면 저장)·BGP(배경색), 기본 형 표에 M 확장자 55개
; 0.10.0: INI 한 줄에 ';' 로 여러 명령, %f = 확장자 뺀 경로
; 0.9.9: 사용자 메뉴 항목 추가를 프로그램 고르기(파일 선택 팝업)로
; 0.9.8: 사용자 메뉴 Ctrl+G: INI 의 MENU 줄
; 0.9.7: Quick-Dir 를 팝업으로, 기능키 안내를 F1~F5 / Shift 누르면 F6~F10 으로, 목록 칸을 패널 폭에 맞춤
; 0.9.6: SCREEN 7 모드·/7 /0 옵션·ESC DOS 화면·빠른 폴더 읽기·F9 드라이브 풀다운(F4 = 화면 방식)
; 0.9.5: 파일 목록에서 시간 열 뺌, 날짜만 표시
;     프로그램 추가 때 파일 선택 창.
;     확장자 하나에 프로그램 여럿(INI 같은 확장자 줄 여러 개): Enter 가 풀다운으로 고르게, Ctrl+F3 으로 목록 관리.
;     Ctrl+F3 외부 뷰어 선택(INI 의 VIEW 줄), F2 가 외부 뷰어를 실행.
;     트리를 선(├ └ │)으로 그림.
;     Ctrl+W 트리 추가.
;     Ctrl+F 파일 찾기 추가.
;     F3 편집기 추가(매퍼 쪽 하나, 16000 바이트)
;     F2 텍스트 보기.
;     모든 질문·입력을 팝업 창으로(popup_core): 삭제·덮어쓰기·이동·읽기전용·드라이브·이름 입력·명령줄.
;     팝업 창(popup_yn) 추가, F5 확인에 사용.F5 BASIC 에서 CALL SYSTEM 하면 X86FC 로 자동 복귀(H.GONE 훅).F8 풀다운 정렬(이름·확장자·날짜), F5 = MSX-BASIC, F4 = 이름만.
;     마스터님 키 배정 적용: F1~F10, Ctrl+F1/F2, 정렬 F8, Home=루트, Ctrl+Enter/Up/Down.
;     명령줄 '/', Quick-Dir ^B,RETURN 으로 COM·BAT·X86FC.INI 연결 실행, 끝나면 같은 줄로 복귀)
;
; C 단계: 고르기(SPACE·^A), 복사(INS) — 파일·폴더(하위 폴더까지) 복사, 긴 이름(원본 조각 그대로 옮김)·8.3 겹침 별칭,
;        폴더 겹치기·자기 안으로 복사 막기까지. 삭제(DEL) — 확인 질문, 읽기 전용 확인, 폴더 처리, 긴 이름 조각 지움.
;        폴더 만들기(^F)·이름 바꾸기(^E) — 이름 입력(ASCII), 8.3 별칭 정하기, 긴 이름 조각 만들기·옛 조각 지우기.
;        이동은 아직 없음.
;
; B 단계: 긴 파일 이름(VFAT) 읽기 — 디렉터리 원본 섹터를 직접 읽어 마지막 줄·앞글자 찾기에 씀 (아래 "긴 이름" 참고).
;
; A 단계가 하는 일:
;   - MSX2 이상 + MSX-DOS2/Nextor 확인, 80칸 글자 화면(SCREEN 0), 두 패널
;   - 디렉터리 읽기(DOS2 _FFIRST/_FNEXT, 8.3 이름), 폴더 먼저 + 이름 순 정렬
;   - ↑↓ 커서, ←→ 쪽 이동, HOME 루트, TAB 패널 바꿈, RETURN 폴더 들어가기
;     BS 상위 폴더(또는 입력 글자 지우기), ^D 드라이브 고르기, 글자 치면 그 이름으로 찾아감, ESC 로 DOS 화면
;   - 커서 줄 강조는 글자 화면의 깜박임 기능 (R#12 = 강조 색, R#13 = 켜짐·꺼짐 시간) 사용 (V9938/V9958/V9968 공통 규격)
;
; 메모리: 코드·글꼴 0100h~, 작업 영역(BSS)은 그 뒤 (아래 equ). 모두 본체 TPA 안
; 조립: pasmo x86fc.asm X86FC.COM
; ======================================================================
BDOS            equ     0005h
CALSLT          equ     001Ch
RDSLT_ADDR      equ     000Ch
EXPTBL          equ     0FCC0h

; ---- BIOS 번지 (슬롯 간 호출은 CALSLT 로만) ----
CHGMOD          equ     005Fh
CHGCLR          equ     0062h
ERAFNK          equ     00CCh
DSPFNK          equ     00CFh
CHGET           equ     009Fh
CHSNS           equ     009Ch

; ---- 시스템 변수 ----
LINL40          equ     0F3AEh
CRTCNT          equ     0F3B1h
FORCLR          equ     0F3E9h
BAKCLR          equ     0F3EAh
BDRCLR          equ     0F3EBh
CNSDFG          equ     0F3DEh
CSRSW           equ     0FCA9h
SCRMOD          equ     0FCAFh
JIFFY           equ     0FC9Eh
PUTPNT          equ     0F3F8h
GETPNT          equ     0F3FAh

; ---- DOS 함수 ----
F_TERM0         equ     00h
F_CONOUT        equ     02h
F_SETDTA        equ     1Ah
F_CURDRV        equ     19h
F_RDABS         equ     2Fh
F_WRABS         equ     30h
F_RDDRV         equ     73h                    ; Nextor: 32 비트 섹터 번호로 읽기 (A=드라이브, B=개수, HL:DE=섹터)
F_WRDRV         equ     74h
F_OPEN          equ     43h
F_CREATE        equ     44h
F_CLOSE         equ     45h
F_READ          equ     48h
F_WRITE         equ     49h
F_DELETE        equ     4Dh
F_RENAME        equ     4Eh
F_MOVE          equ     4Fh
F_ATTR          equ     50h
F_FTIME         equ     51h             ; (이름으로 날짜·시간)
F_HFTIME        equ     56h             ; 핸들로 날짜·시간 (복사 때 원본 날짜·시간을 그대로 옮김)
F_FLUSH         equ     5Fh
E_EOF           equ     0C7h            ; 파일 끝
F_FFIRST        equ     40h
F_FNEXT         equ     41h
F_GETCD         equ     59h
F_EXPLAIN       equ     66h
F_DEFER         equ     64h
K_QD            equ     02h
K_FIND          equ     06h
K_TREE          equ     17h
F_SELD          equ     0Eh
F_CHDIR         equ     5Ah
F_GENV          equ     6Bh
F_SENV          equ     6Ch
F_DOSVER        equ     6Fh
E_NOFIL         equ     0D7h            ; 존재하지 않는 파일

; ---- 키 ----
K_TAB           equ     09h
K_RET           equ     0Dh
K_ESC           equ     1Bh
K_BS            equ     08h
K_HOME          equ     0Bh
K_RIGHT         equ     1Ch
K_LEFT          equ     1Dh
K_UP            equ     1Eh
K_DOWN          equ     1Fh
K_CTRLD         equ     04h
K_INS           equ     12h             ; 복사
K_DEL           equ     7Fh             ; DEL 키
K_MOVE          equ     18h             ; SELECT 키(^X) = 이동 (확정 키)
K_MKDIR         equ     06h             ; ^F = 폴더 만들기 (Shift+F2 로 배정됨)
K_REN           equ     05h             ; ^E = 이름 바꾸기 (Shift+F1 로 배정됨)
IN_MAX          equ     100             ; 입력 이름 최대 길이
K_CTRLA         equ     01h             ; 모두 고르기·해제
K_SPACE         equ     20h             ; 입력 글자 앞을 한 칸 고르기·해제
TYPE_TIMEOUT    equ     90              ; 입력 글자를 기다리는 시간 (1/60 초 단위)

; ---- 크기 ----
MAXE            equ     190; 패널 하나의 항목 수 한도
ENSZ            equ     24              ; 항목 하나의 크기
PSZ             equ     84              ; 패널 상태 하나의 크기
LISTROWS        equ     18              ; 패널의 목록 줄 수. 줄 0 = 경로, 1 = 가로선, 2~19 = 목록, 20 = 가로선, 21 = 정보(긴 이름 + 개수), 22 = 가로선(전체), 23 = 기능키 안내
LIST_Y          equ     2               ; 목록 시작 줄
ROW_LNAME       equ     LISTROWS+LIST_Y+1       ; 긴 이름 줄
ROW_STAT        equ     ROW_LNAME               ; 정보 줄 (긴 이름 줄과 같은 줄)
ROW_BOT         equ     ROW_STAT+1              ; 기능키 안내 위 가로선
ROW_FKEY        equ     ROW_BOT+1               ; 기능키 안내 줄
PANW            equ     39              ; 패널 폭 (왼쪽 0~38, 세로 구분선 39, 오른쪽 40~78)
CH_VBAR         equ     01h             ; 직접 만든 글자 모양 (패턴 표에 그림)
CH_HBAR         equ     02h
CH_CROSS        equ     03h
CH_TL           equ     05h
CH_TR           equ     06h
CH_BL           equ     07h
CH_BR           equ     08h
CH_TEE          equ     09h
CH_TUP          equ     04h            ; 위로만 이어지는 T 모양 (┴)
FNKSTR          equ     0F87Fh          ; 기능키 글자 영역 (기종마다 16 바이트)
; 기능키 글자를 하나씩 바꿔 둠 (BIOS 가 80h 이상 글자는 안 보여 주지 않아도 되도록 따로 처리)
K_F1            equ     0Eh
K_F2            equ     0Fh
K_F3            equ     10h
K_F4            equ     11h
K_F5            equ     13h
K_F6            equ     14h
K_F7            equ     15h
K_F8            equ     16h
K_F9            equ     1Ah
K_F10           equ     19h
SRCHATTR        equ     37h             ; 찾을 속성: 읽기전용(01h)·숨김(02h)·시스템(04h)·폴더(10h)·보관(20h) 모두

; ---- 작업 영역 (BSS) ----
BSS             equ     code_end+16        ; 코드·글꼴 뒤 (아래 code_end 검사). 모두 본체 TPA 안
OVSZ            equ     3328           ; 오버레이 창 크기 (X86FC.OVL 한 칸 = OVSZ 바이트)
OVN             equ     6                       ; (0.12.14) 오버레이 칸 수 (X86FC.OVL 앞부분). 그 뒤 16바이트 표식 + 글꼴 11172자 x 18B + EUC-KR 표
HGB             equ     OVN*OVSZ+16             ; OVL 안 글꼴 시작 위치
OVBASE          equ     BSS             ; 오버레이 창 (코드 바로 뒤 — 편집기가 8000h 쪽을 매퍼로 바꿔 끼우므로 8000h 아래에 둠)
PTR0            equ     OVBASE+OVSZ
PTR1            equ     PTR0+MAXE*2
ENT0            equ     PTR1+MAXE*2
ENT1            equ     ENT0+MAXE*ENSZ
FIB             equ     fibmem          ; 64 바이트 (프로그램 파일 정보)
OLDFNK          equ     ENT1+MAXE*ENSZ  ; 시작할 때의 기능키 글자 영역 160 바이트 (끝낼 때 되돌림)
LNSZ            equ     2048          ; 패널마다 긴 이름 저장소
LFN_MAX         equ     100             ; 긴 이름 하나에서 보관하는 글자 수 한도
LN0             equ     OLDFNK+160
LN1             equ     LN0+LNSZ
SECBUF          equ     LN1+LNSZ        ; 기본 섹터 하나 (512)
FATBUF          equ     SECBUF+512      ; FAT 섹터 둘 (1024)
LFNBUF          equ     FATBUF+1024     ; 긴 이름 조립 칸 (272)
LFNBUF_SZ       equ     522                     ; 긴 이름 조립 칸: UCS-2 원값 26 바이트 x 20 조각 + 끝 2
LFRAW           equ     LFNBUF+LFNBUF_SZ      ; 긴 이름 조각 원본 (항목 32 바이트 x 20)
MAXD            equ     6               ; 폴더 복사에서 들어갈 수 있는 최대 깊이
LVSZ            equ     132             ; 한 단계 저장: 원본 경로 64 + 대상 경로 64 + 반복 번호 2 (+2)
LVSTK           equ     LFRAW+640
ENVBUF          equ     LVSTK+MAXD*LVSZ ; 환경 변수 X86FC 글 (저장·읽기용, 256)
RUNPATH         equ     ENVBUF+256      ; 실행할 프로그램 경로 (64)
SELFPATH        equ     RUNPATH+64      ; X86FC.COM 자신의 경로 (64)
CMDBUF          equ     SELFPATH+64     ; 명령 틀을 펼친 글 (256)
PROGBUF         equ     CMDBUF+256      ; 실행할 프로그램 경로 (64)
TAILBUF         equ     PROGBUF+64      ; 명령줄 (0080h 에 넣을 128, 조립 중 넘침 대비 256)
CPBUF           equ     TAILBUF+256    ; 복사 버퍼 (실행 중 남는 만큼 씀, 최소 2KB)
BSS_END         equ     CPBUF+2048
ILBUF           equ     ENVBUF                  ; (0.12.14) INI 줄 버퍼 (256)
IRBUF           equ     SECBUF                  ; (0.12.14) INI 읽기 버퍼 (512)

; ---- 패널 상태 (IX 가 가리킴) ----
PS_COUNT        equ     0               ; w 항목 수
PS_TOP          equ     2               ; w 화면 맨 위 항목 번호
PS_CUR          equ     4               ; w 커서 항목 번호
PS_PATH         equ     8               ; 64 바이트
; ASCIIZ  "A:\DIR\SUB"
PS_PTR          equ     72              ; w 정렬된 포인터 배열
PS_ENT          equ     74              ; w 항목 배열
PS_LNB          equ     76              ; w 긴 이름 저장소 시작
PS_LNP          equ     78              ; w 긴 이름 저장소의 다음 빈 자리
PS_SEL          equ     80              ; w 고른(표시한) 항목 수
; ---- 항목 (24 바이트) ----
EN_NAME         equ     0               ; 8
EN_EXT          equ     8               ; 3
EN_ATTR         equ     11
EN_TIME         equ     12              ; w
EN_DATE         equ     14              ; w
EN_SIZE         equ     16              ; dw
EN_FLAG         equ     20              ; 비트 0 = 고름(표시)
EN_LFN          equ     21              ; w 긴 이름(ASCIIZ) 번지, 0 = 없음

                org     0100h

start:
                ; ---- MSX-DOS2 확인 ----
                ld      ix,0
                ld      c,F_DOSVER
                call    BDOS
                or      a
                jp      nz,not_dos2
                ld      a,b
                cp      2
                jp      c,not_dos2

                call    chk_back                ; "X86FC /B" (AUTOEXEC.BAT 끝줄용): 저장된 상태가 없으면 조용히 끝냄
                ; ---- MSX 종류·VDP 포트 (BIOS 값) ----
                ld      hl,002Dh
                call    rd_main
                ld      (msx_ver),a
                or      a
                jp      z,not_msx2
                ld      hl,0007h
                call    rd_main
                ld      (vdp_dat),a
                inc     a
                ld      (vdp_cmd),a

                ; ---- 메모리가 충분한지 (TPA 위쪽 = BDOS 아래) ----
                ld      hl,(0006h)
                ld      de,-0400h
                add     hl,de
                ld      de,BSS_END
                or      a
                sbc     hl,de
                jp      c,no_mem

                ; ---- 디스크 오류 처리기 (화면을 더럽히지 않게) ----
                ld      de,disk_err
                ld      c,F_DEFER
                call    BDOS

                call    panels_init
                call    restore_state
                call    mig_chk                 ; (0.12.14) 옛 SET·MNU·QD·HIS 가 있으면 INI 로 옮김
                call    s7_ini                  ; INI 의 SCREEN·COLOR 줄
                call    scr_init
                call    draw_all
                call    main_loop
                ld      a,1
                ld      (dos_rest),a
                call    scr_exit
                ld      c,F_TERM0
                jp      BDOS

not_dos2:       ld      hl,txt_need2
                jr      fail_msg
not_msx2:       ld      hl,txt_needmsx2
                jr      fail_msg
no_mem:         ld      hl,txt_nomem
fail_msg:       call    puts
                ld      c,F_TERM0
                jp      BDOS

; 디스크 오류 처리기: 돌려주는 값 3 = 실패 처리 (호출한 함수가 오류 코드를 돌려줌)
disk_err:       ld      a,3
                ret

; ======================================================================
; 주 루프
; ======================================================================
main_loop:
ml_next:        call    getkey
                or      a
                jp      z,ml_idle               ; 0 = 입력 글자 시간 초과
                ld      hl,0
                ld      (msg_p),hl              ; 안 눌린 상태의 알림 글자 지움
                cp      K_ESC
                jp      z,ml_esc
                cp      K_TAB
                jp      z,ml_tab
                cp      K_UP
                jp      z,ml_up
                cp      K_DOWN
                jp      z,ml_down
                cp      K_LEFT
                jp      z,ml_pgup
                cp      K_RIGHT
                jp      z,ml_pgdn
                cp      K_HOME
                jp      z,ml_home
                cp      K_RET
                jp      z,ml_ret
                cp      K_BS
                jp      z,ml_bs
                cp      K_CTRLD
                jp      z,ml_drive
                cp      K_INS
                jp      z,ml_copy
                cp      K_DEL
                jp      z,ml_del
                cp      K_MOVE
                jp      z,ml_move
                cp      K_FIND                  ; ^F 파일 찾기
                jp      z,ml_find
                cp      K_TREE                  ; ^W 트리 보기
                jp      z,ml_tree
                cp      '/'                    ; 명령줄 (임시 키)
                jp      z,ml_cmd
                cp      07h                     ; Ctrl+G = 사용자 메뉴 (INI 의 MENU 줄)
                jp      z,ml_umenu
                cp      K_QD                    ; Quick-Dir (임시 키 ^B)
                jp      z,ml_qd
                cp      0F3h                    ; CTRL+F3 (getkey 가 만들어 줌)
                jp      z,ml_cf3                ; CTRL+F3 = 파일 형식별 외부 뷰어 선택
                cp      K_F1
                jp      z,ml_f1
                cp      K_F2
                jp      z,ml_f2
                cp      K_F3
                jp      z,ml_f3
                cp      K_F4
                jp      z,ml_mode               ; F4 Screen = 화면 방식 (글자 ↔ SCREEN 7)
                cp      K_F5
                jp      z,ml_f5
                cp      K_F6
                jp      z,ml_ren
                cp      K_F7
                jp      z,ml_mkdir
                cp      K_F8
                jp      z,ml_sort
                cp      K_F9                    ; F9 = 활성 패널의 드라이브 바꾸기 (풀다운)
                jp      z,ml_dsel
                cp      K_F10
                ret     z                       ; F10 = 끝
                jp      ml_chars
ml_chars:       cp      K_CTRLA
                jp      z,ml_selall
                cp      K_SPACE
                jr      nz,ml_ch2
                ld      b,a                     ; 타이핑 찾기: 입력 글자가 있으면 고르고, 없으면 이름 글자
                ld      a,(typelen)
                or      a
                ld      a,b
                jr      nz,ml_ch2
                call    cur_ix
                call    sel_toggle
                jp      ml_redraw
ml_ch2:         cp      20h
                jp      c,ml_next
                cp      7Fh
                jp      nc,ml_next
                call    type_char
                jp      ml_redraw
ml_selall:      call    cur_ix
                call    sel_all
                jp      ml_redraw

ml_copy:        call    typeclr
                call    do_copy
                call    draw_all
                jp      ml_next
ml_del:         call    typeclr
                call    do_delete
                call    draw_all
                jp      ml_next
ml_move:        call    typeclr
                call    do_move
                call    draw_all
                jp      ml_next
ml_mkdir:       call    typeclr
                call    do_mkdir
                call    draw_all
                jp      ml_next
ml_ren:         call    typeclr
                call    do_rename
                call    draw_all
                jp      ml_next
SNSMAT          equ     0141h
RDPSG           equ     0096h
WRTPSG          equ     0093h
; CTRL 이 눌려 있으면 Z (키보드 행 6, 비트 1)
ctrl_down:      push    ix
                ld      a,6
                ld      ix,SNSMAT
                call    bios
                pop     ix
                bit     1,a
                ret
ml_f1:          jp      ml_help                 ; F1 = 도움말
ml_f3:          call    do_edit                 ; F3 = 보기 + 편집
                call    clear_screen
                call    draw_all
                jp      ml_next
ml_find:        call    typeclr
                call    do_find
                call    clear_screen
                call    draw_all
                jp      ml_next
ml_tree:        call    typeclr
                call    do_tree
                call    clear_screen
                call    draw_all
                jp      ml_next
ml_cf3:         call    typeclr
                call    do_setview
                call    draw_all
                jp      ml_next
ml_f2:          call    do_view                 ; F2 = 텍스트 보기 전용
                call    clear_screen
                call    draw_all
                jp      ml_next
ml_dsel:        call    typeclr
                call    draw_all
                call    cur_ix
                call    drive_prompt
                call    draw_all
                jp      ml_next
ml_sort:        call    typeclr                 ; F8: 정렬 방식 돌려 가며 (이름 → 확장자 → 크기 → 날짜)
                call    sort_menu               ; 풀다운에서 고른 방식
                cp      0FFh
                jr      nz,ms_set
                call    draw_all
                jp      ml_next
ms_set:         ld      (sort_mode),a
                call    ini_sort                ; 고른 정렬을 INI 에 저장 (다음에도 유지)
                ld      ix,pstate
                call    resort_panel
                ld      ix,pstate+PSZ
                call    resort_panel
                ld      a,(sort_mode)
                add     a,a
                ld      e,a
                ld      d,0
                ld      hl,sort_msgs
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ld      (msg_p),de
                call    draw_all
                jp      ml_next
sort_menu:      ld      a,5                     ; (0.12.15) F8 정렬 풀다운은 오버레이 5
                ld      b,3
                jp      ov_go

; IX = 패널: 커서 항목을 기억해 두고 다시 정렬한 뒤 그 항목으로
resort_panel:   ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                ret     z
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                call    disp_name
                ld      hl,nmbuf
                ld      de,upname
                call    strcpy_nul
                call    dir_sort
                jp      find_exact
; 활성 패널을 드라이브 루트로 (HOME)
go_root:        ld      (ix+PS_PATH+3),0
                jp      panel_load
; F5: MSX-BASIC 으로 (DOS 의 BASIC 명령). BASIC 에서 _SYSTEM 하면 DOS 로 돌아옴 → X86FC 를 다시 실행하면 같은 자리로 복귀
ml_f5:          call    typeclr                 ; F5 = MSX-BASIC (DOS 의 BASIC 명령을 임시 배치로 실행)
                ld      hl,txt_f5q              ; 먼저 팝업으로 물어봄
                call    popup_yn
                jr      z,f5_go
                call    draw_all
                jp      ml_next
f5_go:          xor     a
                ld      (cl_tmp),a
                call    f5_bas                  ; BASIC 시작 프로그램(X86FCB.BAS) 만들고 inbuf = "BASIC <그 경로>"
                ret     c
                ld      a,2                     ; 2 = F5: 돌아와서 X86FCB.BAS 를 지움
                ld      (cl_tmp),a
                call    dc_make
                call    draw_all
                jp      ml_next
; ======================================================================
; F2 텍스트 보기 (보기 전용): 파일을 2000 바이트씩 읽어 가며 22 줄(80 칸)을 보여 줌 (긴 줄은 80 칸에서 접음)
;  위/아래 = 한 줄, 왼쪽/오른쪽/SPACE = 한 쪽, HOME = 처음, B = 끝, ESC·F10·Q = 닫기
; ======================================================================
F_SEEK          equ     4Ah
VBUF            equ     SECBUF
; ---- 오버레이 1 의 진입점 (본체에는 껍데기만 남김) ----
do_view:        ld      a,4                     ; 보기 = 오버레이 4 (7H 깃발을 끄지 않고 그림)
                ld      b,0
                jp      ov_go
do_edit:        ld      a,1
                ld      b,0
                jp      ov_go
; (0.12.27) 연결 줄의 내부 명령: HL = 명령 틀. #VIEW = F2 와 같은 내부 보기, #EDIT = F3 와 같은 내부 편집기.
; 같은 진입점(do_view·do_edit)을 쓰고 커서 항목이 대상 파일. 알 수 없는 # 명령은 팝업. 돌아오면 목록을 다시 그림.
rc_int:         ld      de,txt_i_view
                push    hl
                call    int_cmp
                pop     hl                      ; (int_cmp 는 HL 을 전진시키므로 되돌림)
                jr      z,ri_v
                ld      de,txt_i_edit
                call    int_cmp
                jr      z,ri_e
                ld      hl,txt_unkint
                ld      de,txt_ppok
                call    popup_ask
                jp      draw_all
ri_v:           call    do_view
                jr      ri_x
ri_e:           ld      a,1                     ; INI 에 적은 #EDIT 는 사용자가 정한 연결이니 "글 형식이 아님" 질문을 건너뜀
                ld      (ed_force),a
                call    do_edit
                xor     a
                ld      (ed_force),a
ri_x:           call    clear_screen
                jp      draw_all
; HL = 명령 글, DE = 맞출 글(대문자, NUL 끝) → Z = 같음 (뒤가 끝이거나 공백)
int_cmp:        ld      a,(de)
                or      a
                jr      z,ic_end
                ld      c,a
                ld      a,(hl)
                cp      'a'
                jr      c,ic_u
                and     0DFh
ic_u:           cp      c
                ret     nz
                inc     hl
                inc     de
                jr      int_cmp
ic_end:         ld      a,(hl)
                or      a
                ret     z
                cp      ' '
                ret
ed_force:       db      0
txt_i_view:     db      "#VIEW",0
txt_i_edit:     db      "#EDIT",0
txt_unkint:     db      "Unknown internal command",0
txt_ppok:       db      "[Enter] OK",0
; ---- 오버레이 0 의 진입점 (본체에는 껍데기만 남김) ----
do_find:        ld      a,0
                ld      b,0
                jp      ov_go
do_tree:        ld      a,0
                ld      b,1
                jp      ov_go
; ======================================================================
; 외부 뷰어 (X86FC.INI 의 "VIEW 확장자 = 명령" 줄): F2 는 그 확장자에 뷰어가 정해져 있으면 그것을 실행(복귀 지원),
; 없으면 내부 텍스트 보기.  Ctrl+F3 = 지금 파일 확장자의 외부 뷰어를 입력창에서 정함(빈 칸이면 지움) → INI 에 저장.
; ======================================================================
; "view" + 공백들 건너뜀 (HL 전진) → Z = 맞음
fl_eatv:        ld      de,txt_vw
                ld      b,4
ev_l:           ld      a,(hl)
                or      20h
                ld      c,a
                ld      a,(de)
                cp      c
                ret     nz
                inc     hl
                inc     de
                djnz    ev_l
                ld      a,(hl)
                cp      ' '
                jr      z,ev_s
                cp      9
                ret     nz
ev_s:           inc     hl
                ld      a,(hl)
                cp      ' '
                jr      z,ev_s
                xor     a
                ret
; 줄 시작 HL 이 "VIEW <sv_ext>" 줄인가 → Z
sv_match:       ld      a,(hl)
                cp      ' '
                jr      z,svm_n
                cp      9
                jr      nz,svm_v
svm_n:          inc     hl
                jr      sv_match
svm_v:          ld      de,sv_ext
                ld      b,3
svm_c:          ld      a,(hl)
                cp      'a'
                jr      c,svm_u
                cp      'z'+1
                jr      nc,svm_u
                and     0DFh
svm_u:          cp      9
                jr      z,svm_sp
                cp      '='
                jr      z,svm_sp
                cp      13
                jr      z,svm_sp
                cp      10
                jr      z,svm_sp
                or      a
                jr      z,svm_sp
                ld      c,a
                ld      a,(de)
                cp      c
                ret     nz
                inc     hl
                inc     de
                djnz    svm_c
                jr      svm_e
svm_sp:         ld      a,(de)                  ; 줄의 확장자가 끝났으면 남은 칸은 공백이어야 함
                cp      ' '
                ret     nz
                inc     de
                djnz    svm_sp
svm_e:          ld      a,(hl)
                cp      ' '
                ret     z
                cp      '='
                ret     z
                cp      9
                ret
vl_prep:        ld      hl,(en_p)
                ld      de,EN_EXT
                add     hl,de
                ld      de,sv_ext
                ld      bc,3
                ldir
                jp      vl_load
; ---- Ctrl+F3: 이 파일 형식으로 열 프로그램 목록 ("Open with"): 한 확장자에 프로그램을 여러 개 둘 수 있음 ----
;  X86FC.INI 에 같은 확장자의 VIEW 줄을 여러 줄 두면 위에서부터 목록. 맨 위가 기본(F2 가 실행).
;  목록 팝업: ↑↓ 고르기, RETURN 실행, INS 추가, DEL 지움, D 기본으로(맨 위로), ESC 닫기
VLBUF           equ     LVSTK
VLREC           equ     80
VLMAX           equ     8
; ---- 오버레이 3 의 진입점 (본체에는 껍데기만 남김) ----
vl_manage:      ld      a,3
                ld      b,0
                jp      ov_go
do_setview:     ld      a,3
                ld      b,1
                jp      ov_go
; A = 번호 → DE = 그 자리 (VLBUF + 번호 × 80), HL 보존
vl_slot:        push    hl
                ld      l,a
                ld      h,0
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; ×16
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,hl                   ; ×64
                add     hl,de                   ; ×80
                ld      de,VLBUF
                add     hl,de
                ex      de,hl
                pop     hl
                ret
; INI 를 읽어 이 확장자의 VIEW 줄들을 VLBUF 에 모음
vl_load:        xor     a
                ld      (vl_n),a
                call    il_open                 ; (0.12.14) INI 전체를 줄 단위로 읽음
                ret     nz
vll_nx:         call    il_next
                jr      z,vll_end
                ld      hl,ILBUF
                call    sv_match
                jr      nz,vll_nx
                ld      a,(vl_n)
                cp      VLMAX
                jr      nc,vll_end
vll_eq:         ld      a,(hl)
                or      a
                jr      z,vll_nx
                cp      '='
                jr      z,vll_f
                inc     hl
                jr      vll_eq
vll_f:          inc     hl
vll_sp:         ld      a,(hl)
                cp      ' '
                jr      nz,vll_c
                inc     hl
                jr      vll_sp
vll_c:          ld      a,(vl_n)
                call    vl_slot
                ld      b,VLREC-2
vll_cp:         ld      a,(hl)
                or      a
                jr      z,vll_e
                cp      ';'
                jr      z,vll_semi              ; ';' 로 이은 명령은 따로 한 칸씩
                ld      (de),a
                inc     hl
                inc     de
                djnz    vll_cp
vll_e:          xor     a
                ld      (de),a
                ld      a,(vl_n)
                inc     a
                ld      (vl_n),a
                jr      vll_nx
vll_end:        jp      il_close
vll_semi:       xor     a
                ld      (de),a
                ld      a,(vl_n)
                inc     a
                ld      (vl_n),a
                cp      VLMAX
                jr      nc,vll_end
                inc     hl
vll_ss:         ld      a,(hl)
                cp      ' '
                jr      nz,vll_s2
                inc     hl
                jr      vll_ss
vll_s2:         or      a
                jr      z,vll_nx
                jp      vll_c
; ---- 프로그램 찾아서 고르기 (파일 선택 창): 폴더를 오가며 파일을 고름 ----
;  첫 줄 "<type a command...>" = 직접 입력, ".." = 위 폴더, 폴더는 RETURN 으로 들어감, 파일은 RETURN 으로 고름.
;  Up/Down 이동, Left/Right 한 쪽, BS 위 폴더, \ 루트, 글자 = 그 글자로 시작하는 항목, ESC 취소.
;  돌아올 때 A = 0 고름(pk_path), 1 직접 입력, 2 취소
PKMAX           equ     138
pick_file:      call    draw_all
                call    cur_ix
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      de,pk_dir
                call    copy_z
                xor     a
                ld      (de),a
pk_scan:        call    pk_read
                xor     a
                ld      (pk_sel),a
                ld      (pk_top),a
pk_draw:        ld      de,errbuf               ; 제목 = 안내 + 지금 폴더
                ld      hl,txt_pkt
                call    copy_z
                ld      hl,pk_dir
                call    copy_z
                xor     a
                ld      (de),a
                xor     a
                ld      (pk_r),a
                ld      a,14
                ld      (pk_re),a               ; 처음·폴더가 바뀔 때만 14 줄 모두 그림 (↑↓ 는 pk_upd 로 바뀐 줄만)
                ld      a,(pk_sel)
                ld      (pk_old),a
                call    pkd_l
                jp      pk_key
pkd_l:          ld      hl,qd_tmp               ; 한 줄(60 칸) 바탕
                ld      a,(pk_r)
                or      a
                jr      nz,pkd_n0
                ld      c,CH_TL
                ld      d,CH_HBAR
                ld      e,CH_TR
                jr      pkd_f
pkd_n0:         cp      13
                jr      nz,pkd_n5
                ld      c,CH_BL
                ld      d,CH_HBAR
                ld      e,CH_BR
                jr      pkd_f
pkd_n5:         ld      c,CH_VBAR
                ld      d,' '
                ld      e,CH_VBAR
pkd_f:          ld      (hl),c
                inc     hl
                ld      b,58
pkd_fl:         ld      (hl),d
                inc     hl
                djnz    pkd_fl
                ld      (hl),e
                ld      a,(pk_r)
                cp      1
                jr      nz,pkd_x2
                ld      hl,errbuf
                jr      pkd_txt
pkd_x2:         cp      12
                jr      nz,pkd_x3
                ld      hl,txt_pkh
                jr      pkd_txt
pkd_x3:         cp      2
                jr      c,pkd_wr
                cp      13
                jr      z,pkd_wr                ; 맨 아래 테두리 줄에는 항목 글을 쓰지 않음
                sub     2
                ld      b,a                     ; 목록 줄 번호 0..9
                ld      a,(pk_top)
                add     a,b
                ld      (pk_i),a
                ld      b,a
                ld      a,(pk_n)
                cp      b
                jr      z,pkd_wr
                jr      c,pkd_wr
                ld      a,(pk_i)
                call    pk_adr
                ld      a,(hl)
                ld      (pk_at),a
                inc     hl
                ld      de,qd_tmp+5
                ld      b,50
pkd_nc:         ld      a,(hl)
                or      a
                jr      z,pkd_ne
                ld      (de),a
                inc     hl
                inc     de
                djnz    pkd_nc
pkd_ne:         ld      a,(pk_at)
                cp      10h
                jr      nz,pkd_mk0
                ld      a,'\'
                ld      (de),a
pkd_mk0:        ld      hl,pk_sel
                ld      a,(pk_i)
                cp      (hl)
                ld      a,' '
                jr      nz,pkd_mk
                ld      a,'>'
pkd_mk:         ld      (qd_tmp+3),a
                jr      pkd_wr
pkd_txt:        ld      de,qd_tmp+3             ; HL 글을 54 칸 안에서 복사
                ld      b,54
pkd_tc:         ld      a,(hl)
                or      a
                jr      z,pkd_wr
                ld      (de),a
                inc     hl
                inc     de
                djnz    pkd_tc
pkd_wr:         ld      a,(pk_r)
                add     a,5
                call    row_addr
                ld      de,10
                add     hl,de
                ld      de,qd_tmp
                ld      b,60
                ld      a,12                    ; (SCREEN 7) 팝업 색
                ld      (s7_pair),a
                call    scr_writep
                xor     a
                ld      (s7_pair),a
                ld      a,(pk_r)
                inc     a
                ld      (pk_r),a
                ld      hl,pk_re
                cp      (hl)
                jp      c,pkd_l
                ret
pk_key:         call    wait_key
                cp      1Bh
                jr      z,pk_esc
                cp      K_DOWN
                jr      z,pk_dn
                cp      K_UP
                jr      z,pk_up
                cp      K_RIGHT
                jp      z,pk_pd
                cp      K_LEFT
                jp      z,pk_pu
                cp      0Dh
                jp      z,pk_ent
                cp      08h
                jp      z,pk_par
                cp      '\'
                jp      z,pk_root
                cp      20h
                jr      c,pk_key
                jp      pk_ta
pk_esc:         ld      a,2
                ret
pk_dn:          ld      a,(pk_sel)
                inc     a
                ld      hl,pk_n
                cp      (hl)
                jr      nc,pk_key
                ld      (pk_sel),a
                jr      pk_scr
pk_up:          ld      a,(pk_sel)
                or      a
                jr      z,pk_key
                dec     a
                ld      (pk_sel),a
pk_scr:         ld      a,(pk_top)
                ld      (pk_ot),a
                ld      a,(pk_sel)              ; 화면 밖이면 맞춤
                ld      hl,pk_top
                cp      (hl)
                jr      nc,pk_s2
                ld      (hl),a
pk_s2:          ld      a,(pk_top)
                add     a,10
                ld      b,a
                ld      a,(pk_sel)
                cp      b
                jr      c,pk_upd
                ld      a,(pk_sel)
                sub     9
                ld      (pk_top),a
pk_upd:         ld      a,(pk_ot)               ; 목록이 넘어가면 목록 10 줄만, 아니면 예전 줄·새 줄 두 줄만 (상자·제목·안내는 그대로)
                ld      hl,pk_top
                cp      (hl)
                jr      z,pk_u1
                ld      a,2
                ld      (pk_r),a
                ld      a,12
                ld      (pk_re),a
                call    pkd_l
                jr      pk_u2
pk_u1:          ld      a,(pk_old)
                call    pk_row
                ld      a,(pk_sel)
                call    pk_row
pk_u2:          ld      a,(pk_sel)
                ld      (pk_old),a
                jp      pk_key
pk_row:         ld      hl,pk_top               ; A = 항목 번호 → 그 줄만 그림
                sub     (hl)
                add     a,2
                ld      (pk_r),a
                inc     a
                ld      (pk_re),a
                jp      pkd_l
pk_pd:          ld      a,(pk_sel)
                add     a,10
                ld      b,a
                ld      a,(pk_n)
                dec     a
                cp      b
                jr      nc,pk_pd1
                ld      b,a
pk_pd1:         ld      a,b
                ld      (pk_sel),a
                jr      pk_scr
pk_pu:          ld      a,(pk_sel)
                sub     10
                jr      nc,pk_pu1
                xor     a
pk_pu1:         ld      (pk_sel),a
                jp      pk_scr
pk_ent:         ld      a,(pk_sel)
                call    pk_adr
                ld      a,(hl)
                cp      0FFh
                jr      nz,pk_e1
                ld      a,1                     ; 직접 입력
                ret
pk_e1:          cp      10h
                jr      nz,pk_file
                inc     hl
                ld      a,(hl)
                cp      '.'
                jr      z,pk_par                ; ".." → 위 폴더
                push    hl                      ; 폴더로 들어감
                ld      hl,pk_dir
                call    strlen
                ld      e,a
                ld      d,0
                ld      hl,pk_dir
                add     hl,de
                dec     hl
                ld      a,(hl)
                inc     hl
                cp      '\'
                jr      z,pk_e2
                ld      (hl),'\'
                inc     hl
pk_e2:          ex      de,hl
                pop     hl
                call    copy_z
                xor     a
                ld      (de),a
                jp      pk_scan
pk_file:        inc     hl                      ; 파일: 경로 = 폴더 + \ + 이름
                push    hl
                ld      hl,pk_dir
                ld      de,pk_path
                call    copy_z
                dec     de
                ld      a,(de)
                inc     de
                cp      '\'
                jr      z,pk_f2
                ld      a,'\'
                ld      (de),a
                inc     de
pk_f2:          pop     hl
                call    copy_z
                xor     a
                ld      (de),a
                ret                             ; A = 0
pk_par:         ld      hl,pk_dir               ; 위 폴더: 마지막 '\' 에서 자름 (루트면 그대로)
                call    strlen
                cp      4
                jp      c,pk_key
                ld      e,a
                ld      d,0
                ld      hl,pk_dir
                add     hl,de
pk_p1:          dec     hl
                ld      a,(hl)
                cp      '\'
                jr      nz,pk_p1
                ld      de,pk_dir+3
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,pk_p2
                ld      hl,pk_dir+2             ; 루트 "A:\" 는 '\' 를 남김
                inc     hl
pk_p2:          ld      (hl),0
                jp      pk_scan
pk_root:        ld      a,0
                ld      (pk_dir+3),a
                jp      pk_scan
pk_ta:          and     0DFh                    ; 글자 → 그 글자로 시작하는 다음 항목
                ld      (pk_ch),a
                ld      a,(pk_sel)
                inc     a
                ld      b,a
pk_t1:          ld      a,(pk_n)
                cp      b
                jr      z,pk_t2
                jr      c,pk_t2
                ld      a,b
                call    pk_adr
                inc     hl
                ld      a,(hl)
                and     0DFh
                ld      c,a
                ld      a,(pk_ch)
                cp      c
                jr      z,pk_t3
                inc     b
                jr      pk_t1
pk_t3:          ld      a,b
                ld      (pk_sel),a
                jp      pk_scr
pk_t2:          jp      pk_key
; A = 번호 → HL = 항목 자리 (CPBUF + 번호 × 14), BC·DE 보존
pk_adr:         push    bc
                ld      l,a
                ld      h,0
                add     hl,hl
                ld      b,h
                ld      c,l
                add     hl,hl
                add     hl,hl
                add     hl,hl
                or      a
                sbc     hl,bc
                ld      bc,CPBUF
                add     hl,bc
                pop     bc
                ret
; 폴더 pk_dir 의 내용을 CPBUF 에 모음 (0 = 직접 입력, 1 = "..", 폴더들, 파일들)
pk_read:        ld      a,0
                call    pk_adr
                ld      (hl),0FFh
                inc     hl
                ex      de,hl
                ld      hl,txt_pkman
                call    copy_z
                xor     a
                ld      (de),a
                ld      a,1
                ld      (pk_n),a
                ld      hl,pk_dir
                call    strlen
                cp      4
                jr      c,pr_p1
                ld      a,(pk_n)
                call    pk_adr
                ld      (hl),10h
                inc     hl
                ld      (hl),'.'
                inc     hl
                ld      (hl),'.'
                inc     hl
                ld      (hl),0
                ld      hl,pk_n
                inc     (hl)
pr_p1:          xor     a
                ld      (pk_ph),a
pr_ps:          ld      hl,pk_dir
                ld      de,specbuf
                call    copy_z
                dec     de
                ld      a,(de)
                inc     de
                cp      '\'
                jr      z,pr_s
                ld      a,'\'
                ld      (de),a
                inc     de
pr_s:           ld      hl,txt_star
                ld      bc,4
                ldir
                ld      de,specbuf
                ld      b,SRCHATTR
                ld      c,F_FFIRST
                call    bdos_fib
                jr      nz,pr_end
pr_l:           ld      a,(FIB+14)
                and     08h
                jr      nz,pr_n
                ld      a,(FIB+1)
                cp      '.'
                jr      z,pr_n
                ld      a,(FIB+14)
                and     10h
                ld      b,a
                ld      a,(pk_ph)
                or      a
                jr      nz,pr_f
                ld      a,b                     ; 1 차: 폴더만
                or      a
                jr      z,pr_n
                jr      pr_add
pr_f:           ld      a,b                     ; 2 차: 파일만
                or      a
                jr      nz,pr_n
pr_add:         ld      a,(pk_n)
                cp      PKMAX
                jr      nc,pr_n
                call    pk_adr
                ld      (hl),b
                inc     hl
                ex      de,hl
                ld      hl,FIB+1
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,pk_n
                inc     (hl)
pr_n:           ld      c,F_FNEXT
                call    bdos_fib
                jr      z,pr_l
pr_end:         ld      a,(pk_ph)
                or      a
                ret     nz
                ld      a,1
                ld      (pk_ph),a
                jr      pr_ps
txt_pkt:        db      "Select program: ",0
txt_pkh:        db      "Enter OK  BS up  \ root  letter jump  Esc cancel",0
txt_pkman:      db      "<type it...>",0
pk_dir:         ds      64
pk_path:        ds      80
pk_n:           db      0
pk_sel:         db      0
pk_top:         db      0
pk_i:           db      0
pk_r:           db      0
pk_re:          db      0
pk_old:         db      0
pk_ot:          db      0
pk_ph:          db      0
pk_at:          db      0
pk_ch:          db      0

sv_ext:         ds      3
fl_view:        db      0
fl_ok:          db      0

; ---- 팝업 창: 화면 가운데에 테두리 상자를 그리고 "예/아니오"를 물음 (HL = 질문 글) → Z = 예 (Y·RETURN) ----
; 닫은 뒤 화면은 호출한 쪽이 draw_all 로 다시 그림. 다른 확인 질문(삭제·덮어쓰기 등)에도 쓸 수 있음.
popup_yn:       ld      (pp_t1),hl              ; HL = 질문 글 → Z = 예 (Y·RETURN)
                ld      hl,txt_ppyn
                ld      (pp_t3),hl
                ld      hl,0
                ld      (pp_t2),hl
                ld      a,30
                ld      (pp_minw),a
                call    popup_core
                call    wait_key
                and     0DFh
                cp      'Y'
                ret     z
                cp      0Dh
                ret
; HL = 질문 글, DE = 단추 글 → A = 누른 글쇠 (영문자는 대문자로)
popup_ask:      ld      (pp_t1),hl
                ld      (pp_t3),de
                ld      hl,0
                ld      (pp_t2),hl
                ld      a,30
                ld      (pp_minw),a
                call    popup_core
                call    wait_key
                cp      'a'
                ret     c
                cp      'z'+1
                ret     nc
                and     0DFh
                ret
; 팝업 상자(7 줄, 8~14 행): 맨 위 줄 / 빈 줄 / 글 1(왼쪽) / 글 2(왼쪽, 입력칸) / 글 3(가운데, 단추) / 빈 줄 / 맨 아래 줄
; 입력: pp_t1, pp_t2 (0 이면 비움), pp_t3, pp_minw (가장 좁은 너비)
popup_core:     ld      hl,(pp_t1)
                call    strlen
                ld      (pp_n),a
                ld      hl,(pp_t3)
                call    strlen
                ld      hl,pp_n
                cp      (hl)
                jr      c,pc_a
                ld      (hl),a
pc_a:           ld      a,(pp_n)
                add     a,6
                ld      hl,pp_minw
                cp      (hl)
                jr      nc,pc_b
                ld      a,(hl)
pc_b:           cp      76
                jr      c,pc_c
                ld      a,76
pc_c:           ld      (pp_w),a
                ld      b,a
                ld      a,80
                sub     b
                srl     a
                ld      (pp_c),a
                xor     a
                ld      (pp_i),a
pp_l:           ld      a,(pp_one)              ; (0.12.14) 0 이 아니면 그 줄 하나만 그림
                or      a
                jr      z,pp_g
                ld      b,a
                ld      a,(pp_i)
                cp      b
                jp      nz,pp_nx
pp_g:           ld      a,(pp_i)
                or      a
                jr      nz,pp_n0
                ld      c,CH_TL
                ld      d,CH_HBAR
                ld      e,CH_TR
                jr      pp_f
pp_n0:          cp      6
                jr      nz,pp_n5
                ld      c,CH_BL
                ld      d,CH_HBAR
                ld      e,CH_BR
                jr      pp_f
pp_n5:          ld      c,CH_VBAR
                ld      d,' '
                ld      e,CH_VBAR
pp_f:           ld      hl,qd_tmp
                ld      (hl),c
                inc     hl
                ld      a,(pp_w)
                sub     2
                ld      b,a
pp_fl:          ld      (hl),d
                inc     hl
                djnz    pp_fl
                ld      (hl),e
                ld      a,(pp_i)
                cp      2
                jr      nz,pp_x3
                ld      hl,(pp_t1)
                jr      pp_left
pp_x3:          cp      3
                jr      nz,pp_x4
                ld      hl,(pp_t2)
pp_left:        ld      a,h
                or      l
                jr      z,pp_wr
                ld      de,qd_tmp+3
                ld      a,(pp_w)
                sub     6
                ld      b,a
pp_lc:          ld      a,(hl)                  ; 너비 안에서만 복사
                or      a
                jr      z,pp_wr
                ld      (de),a
                inc     hl
                inc     de
                djnz    pp_lc
                jr      pp_wr
pp_x4:          cp      4
                jr      nz,pp_wr
                ld      hl,(pp_t3)
                ld      a,h
                or      l
                jr      z,pp_wr
                push    hl
                call    strlen
                ld      b,a
                ld      a,(pp_w)
                sub     b
                srl     a
                ld      e,a
                ld      d,0
                ld      hl,qd_tmp
                add     hl,de
                ex      de,hl
                pop     hl
                call    copy_z
pp_wr:          ld      a,(pp_i)
                add     a,8
                call    row_addr
                ld      a,(pp_c)
                ld      e,a
                ld      d,0
                add     hl,de
                ld      de,qd_tmp
                ld      a,(pp_w)
                ld      b,a
                ld      a,12                    ; (SCREEN 7) 팝업 색
                ld      (s7_pair),a
                call    scr_writep
                xor     a
                ld      (s7_pair),a
pp_nx:          ld      a,(pp_i)
                inc     a
                ld      (pp_i),a
                cp      7
                jp      c,pp_l
                ret
pp_one:         db      0
il_full:        db      0
txt_ppyn:       db      "[Y] Yes     [N] No",0
pp_t1:          dw      0
pp_t2:          dw      0
pp_t3:          dw      0
pp_minw:        db      30
pp_n:           db      0
pp_w:           db      0
pp_c:           db      0
pp_i:           db      0
; ---- BASIC 으로 갔다가 CALL SYSTEM 하면 X86FC 로 자동 복귀 ----
; BASIC 시작 프로그램이 기계어 조각(아래 hk_s~hk_e)을 BASIC 메모리(B000h)에 심고 H.GONE 훅(FF43h)을 걸어 둠.
; 훅은 실행할 문장이 CALL SYSTEM(또는 _SYSTEM)이면 훅을 원래대로(RET) 돌리고 글쇠 버퍼에 "x86fc"+RETURN 을 넣음 →
; DOS 가 다시 뜬 뒤 COMMAND2 가 그 글을 실행 (글쇠 버퍼가 DOS 재시작 뒤에도 남는 것을 시험으로 확인). X86FC 는 시작할 때 저장된 상태로 복귀.
HB              equ     0B000h
f5_bas:         ld      hl,txt_basname          ; BASIC 시작 프로그램 = X86FC.COM 폴더의 X86FCB.BAS (배치 파일과 같은 폴더)
                call    run_path
                ld      hl,SELFPATH             ; 복귀 때 칠 글 = X86FC 전체 경로 (30 글자까지) + RETURN
                ld      de,hk_txt
                ld      b,30
fb_sp:          ld      a,(hl)
                or      a
                jr      z,fb_sq
                ld      (de),a
                inc     hl
                inc     de
                djnz    fb_sp
fb_sq:          ld      a,13
                ld      (de),a
                inc     de
                xor     a
                ld      (de),a
                ; --- BASIC 글 만들기 (CPBUF)
                ld      de,CPBUF
                ld      hl,bas_t1
                call    copy_z
                ld      hl,hk_e-hk_s-1
                call    put_dec16
                ld      hl,bas_t2
                call    copy_z
                ld      hl,hk_s
                ld      (bd_p),hl
                ld      a,hk_e-hk_s
                ld      (bd_left),a
                ld      hl,100
                ld      (bd_no),hl
fb_dl:          ld      a,(bd_left)
                or      a
                jr      z,fb_dd
                ld      hl,(bd_no)
                call    put_dec16
                ld      hl,bas_t3
                call    copy_z
                ld      a,16
                ld      (bd_cnt),a
fb_nb:          ld      hl,(bd_p)
                ld      a,(hl)
                inc     hl
                ld      (bd_p),hl
                ld      l,a
                ld      h,0
                call    put_dec16
                ld      a,(bd_left)
                dec     a
                ld      (bd_left),a
                jr      z,fb_el
                ld      hl,bd_cnt
                dec     (hl)
                jr      z,fb_el
                ld      a,','
                ld      (de),a
                inc     de
                jr      fb_nb
fb_el:          ld      a,13
                ld      (de),a
                inc     de
                ld      a,10
                ld      (de),a
                inc     de
                ld      hl,(bd_no)
                ld      bc,10
                add     hl,bc
                ld      (bd_no),hl
                jr      fb_dl
fb_dd:          ld      a,26
                ld      (de),a
                inc     de
                ld      hl,-CPBUF
                add     hl,de
                ld      (bd_len),hl
                ; --- 파일로
                push    de
                ld      de,PROGBUF
                xor     a
                ld      b,0
                ld      c,F_CREATE
                call    BDOS
                pop     de
                or      a
                jr      nz,fb_bad
                ld      a,b
                ld      (h_dst),a
                ld      hl,(bd_len)
                ld      de,CPBUF
                ld      a,(h_dst)
                ld      b,a
                ld      c,F_WRITE
                call    BDOS
                ld      a,(h_dst)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                ld      de,inbuf                ; inbuf = "BASIC " + 경로
                ld      hl,txt_basic1
                call    copy_z
                ld      hl,PROGBUF
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,inbuf
                call    strlen
                ld      (in_len),a
                or      a                       ; NC
                ret
fb_bad:         ld      hl,txt_notmp
                ld      (msg_p),hl
                scf
                ret
; ---- BASIC 메모리(B000h)에 올라가 H.GONE 훅(FF43h)에서 실행될 조각 ----
hk_s:           push    af
                push    bc
                push    de
                push    hl
                call    HB+(hk_chk-hk_s)        ; 훅이 불릴 때 HL = 실행하려는 문장(토큰) 자리 (시험으로 확인)
                jr      nz,hk_out
hk_fire:        push    hl                      ; (0.12.23) HL = SYSTEM 바로 뒤의 BASIC 글 위치
                ld      hl,0FF43h               ; 훅을 원래대로 (RET 다섯 개)
                ld      b,5
                ld      a,0C9h
hk_r:           ld      (hl),a
                inc     hl
                djnz    hk_r
                pop     hl
                ; 직접 입력으로 친 맨 몸 "CALL SYSTEM" 이면 줄 끝에 ("X86FC 경로") 를 이어 붙여 CALL SYSTEM("명령") 으로 바꿈:
                ;   DOS2/Nextor 표준 기능 (BASIC 이 그 명령을 DOS 로 실행) 이라 글쇠 버퍼나 AUTOEXEC.BAT 에 기대지 않음
                ld      a,(hl)
                or      a
                jr      nz,hk_kb
                push    hl
                ld      de,-0F41Fh              ; KBUF (직접 입력 줄을 토큰으로 바꾼 버퍼, F41Fh) 안인지: 0 <= 위치 - F41Fh < C0h
                add     hl,de
                ld      a,h
                or      a
                jr      nz,hk_nb
                ld      a,l
                cp      0C0h
                jr      nc,hk_nb
                pop     hl
                ld      (hl),'('
                inc     hl
                ld      (hl),'"'
                inc     hl
                ld      de,HB+(hk_txt-hk_s)
hk_cp:          ld      a,(de)
                cp      13
                jr      z,hk_ce
                ld      (hl),a
                inc     hl
                inc     de
                jr      hk_cp
hk_ce:          ld      (hl),'"'
                inc     hl
                ld      (hl),')'
                inc     hl
                ld      (hl),0
                jr      hk_out
hk_nb:          pop     hl
hk_kb:          ld      hl,HB+(hk_txt-hk_s)     ; 그 밖(줄 뒤에 다른 글이 있거나 프로그램 안): 예전 방식 = 글쇠 버퍼에 경로 + Enter
hk_k:           ld      a,(hl)
                or      a
                jr      z,hk_out
                push    hl
                ld      de,(0F3F8h)             ; PUTPNT 에 한 글자 넣기
                ld      (de),a
                inc     de
                ld      hl,0FBF0h+40
                or      a
                sbc     hl,de
                jr      nz,hk_nw
                ld      de,0FBF0h
hk_nw:          ld      (0F3F8h),de
                pop     hl
                inc     hl
                jr      hk_k
hk_out:         pop     hl
                pop     de
                pop     bc
                pop     af
                ret
hk_chk:         ld      a,(hl)                  ; CALL 토큰(CAh) 또는 '_' 다음에 SYSTEM 이 오는가 (Z = 맞음)
                cp      0CAh
                jr      z,hc1
                cp      '_'
                ret     nz
hc1:            inc     hl
hc2:            ld      a,(hl)
                cp      ' '
                jr      nz,hc3
                inc     hl
                jr      hc2
hc3:            ld      de,HB+(hk_sys-hk_s)
hc4:            ld      a,(de)
                or      a
                ret     z
                ld      c,a
                ld      a,(hl)
                and     0DFh
                cp      c
                ret     nz
                inc     hl
                inc     de
                jr      hc4
hk_sys:         db      "SYSTEM",0
hk_txt:         db      "x86fc",13,0            ; (실행할 때 X86FC 전체 경로로 바꿔 넣음 — 현재 폴더가 바뀌어도 찾도록)
                ds      28
hk_e:
bas_t1:         db      "10 CLEAR 200,&HB000:DEFINT A-Z:FOR I=0 TO ",0
bas_t2:         db      ":READ A:POKE &HB000+I,A:NEXT",13,10
                db      "20 POKE &HFF44,0:POKE &HFF45,&HB0:POKE &HFF43,&HC3:NEW",13,10,0
bas_t3:         db      " DATA ",0
txt_basname:    db      "X86FCB.BAS",0
txt_basic1:     db      "BASIC ",0
bd_p:           dw      0
bd_no:          dw      0
bd_len:         dw      0
bd_left:        db      0
bd_cnt:         db      0
; 명령줄에 /B 가 있고 환경 변수 X86FC(저장된 상태)가 없으면 아무것도 안 하고 끝냄.
; BASIC 에서 _SYSTEM 으로 DOS 가 다시 뜨면 AUTOEXEC.BAT 가 다시 실행되므로, 그 끝줄에 "X86FC /B" 를 두면 X86FC 로 자동 복귀
chk_back:       ld      a,(0080h)
                or      a
                ret     z
                ld      b,a
                ld      hl,0081h
cb_l:           ld      a,(hl)
                cp      '/'
                jr      z,cb_s
                inc      hl
                djnz    cb_l
                ret
cb_s:           inc     hl
                ld      a,(hl)
                and     0DFh
                cp      'B'
                ret     nz
                ld      hl,envn_x86
                ld      de,ENVBUF
                ld      b,255
                ld      c,F_GENV
                call    BDOS
                ld      a,(ENVBUF)
                or      a
                ret     nz
                ld      c,F_TERM0
                jp      BDOS
ml_cmd:         call    typeclr
                call    do_cmd
                call    draw_all
                jp      ml_next
ml_qd:          call    typeclr
                call    quickdir
                call    clear_screen
                call    draw_all
                jp      ml_next
ml_help:        call    typeclr
                call    show_help
                call    draw_all
                jp      ml_next

ml_idle:        call    draw_status
                jp      ml_next

ml_esc:         ld      a,(typelen)
                or      a
                jp      z,ml_dos                ; 입력한 글이 없으면 ESC = DOS 화면 보기 (다시 ESC 로 돌아옴)
                ; (입력한 글이 있으면 그 글을 지움) 입력 글자를 지움
                xor     a
                ld      (typelen),a
                jp      ml_redraw

ml_tab:         call    typeclr
                ld      a,(act)
                xor     1
                ld      (act),a
                call    draw_all
                jp      ml_next
ml_cins:        call    typeclr
                call    do_cmd_ins
                call    draw_all
                jp      ml_next
ml_up:          call    ctrl_down               ; CTRL+위 = 한 쪽 위로
                jp      z,ml_pgup
                call    typeclr
                call    cur_ix
                call    fm_save
                call    move_up
                jp      ml_fast
ml_down:        call    ctrl_down               ; CTRL+아래 = 한 쪽 아래로
                jp      z,ml_pgdn
                call    typeclr
                call    cur_ix
                call    fm_save
                call    move_down
                jp      ml_fast
; 위·아래 한 칸: 화면이 넘어가지 않으면 줄 두 개(예전 커서 줄·새 커서 줄)만 다시 그림
fm_save:        ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                ld      (fm_cur),hl
                ld      l,(ix+PS_TOP)
                ld      h,(ix+PS_TOP+1)
                ld      (fm_top),hl
                ret
ml_fast:        call    cur_ix
                ld      l,(ix+PS_TOP)
                ld      h,(ix+PS_TOP+1)
                ld      de,(fm_top)
                or      a
                sbc     hl,de
                jp      nz,ml_redraw            ; 목록이 넘어감 → 전체
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                ld      de,(fm_cur)
                or      a
                sbc     hl,de
                jp      z,ml_next               ; 안 움직임 (처음·끝)
                ld      hl,(fm_cur)             ; 예전 커서 줄
                ld      de,(fm_top)
                or      a
                sbc     hl,de
                ld      a,l
                call    draw_prow
                ld      l,(ix+PS_CUR)           ; 새 커서 줄
                ld      h,(ix+PS_CUR+1)
                ld      de,(fm_top)
                or      a
                sbc     hl,de
                ld      a,l
                call    draw_prow
                call    dp_rows_done            ; 아래 가로선 + 긴 이름 줄
                call    draw_status
                jp      ml_next
; A = 목록 줄 번호(0~) → 그 줄만 그림 (IX = 패널)
draw_prow:      ld      (dp_i),a
                push    ix
                pop     hl
                ld      de,pstate
                or      a
                sbc     hl,de
                ld      a,h
                or      l
                ld      a,0
                jr      z,dpr_pn
                ld      a,1
dpr_pn:         ld      (pn),a
                ld      b,a
                ld      a,(act)
                cp      b
                ld      a,0
                jr      nz,dpr_ia
                ld      a,1
dpr_ia:         ld      (isact),a
                ld      a,1
                ld      (dp_one),a
                call    dp_row
                xor     a
                ld      (dp_one),a
                ret
fm_cur:         dw      0
fm_top:         dw      0
dp_one:         db      0
ml_pgup:        call    typeclr
                call    cur_ix
                call    page_up
                jp      ml_redraw
ml_pgdn:        call    typeclr
                call    cur_ix
                call    page_down
                jp      ml_redraw
ml_home:        call    typeclr
                call    cur_ix
                call    go_root                 ; HOME = 드라이브 루트로
                jp      ml_redraw
ml_ret:         call    ctrl_down               ; CTRL+ENTER = 고른 파일 이름을 명령줄에 넣음
                jp      z,ml_cins
                call    typeclr
                call    cur_ix
                call    panel_enter
                jp      ml_redraw
ml_bs:          ld      a,(typelen)
                or      a
                jr      z,ml_up_dir
                dec     a
                ld      (typelen),a
                jp      ml_redraw
ml_up_dir:      call    cur_ix
                call    panel_up
                jp      ml_redraw
ml_drive:       call    typeclr
                call    cur_ix
                call    drive_prompt
ml_redraw:      call    cur_ix
                call    draw_panel
                call    draw_status
                jp      ml_next

; ======================================================================
; 키 입력 (BIOS CHGET). 입력 글자가 없고 1.5초가 지나면 0 을 돌려줌 (입력 글자 지움)
; ======================================================================
getkey:
gk_wait:        call    fk_shift
                call    fk_ctrl                ; CTRL+F1~F3 은 BIOS 가 글자로 안 주므로 키보드 행렬을 직접 봄
                or      a
                ret     nz
                ld      ix,CHSNS               ; BIOS 가 물어봄 (기능키 글자 빼고 눌린 것이 있나) Z = 입력 없음
                call    bios
                jr      nz,gk_have
                ld      a,(typelen)
                or      a
                jr      z,gk_wait
                ld      hl,(JIFFY)
                ld      de,(type_t)
                or      a
                sbc     hl,de
                ld      de,TYPE_TIMEOUT
                or      a
                sbc     hl,de
                jr      c,gk_wait
                xor     a
                ld      (typelen),a
                ret                             ; A = 0
gk_have:        ld      ix,CHGET
                jp      bios

; CTRL 을 누른 채 F1~F3 이 눌렸으면 A = 0F1h~0F3h (뗄 때까지 기다림), 아니면 0
fk_ctrl:        ld      a,6
                ld      ix,SNSMAT
                call    bios
                bit     1,a                     ; CTRL (0 = 눌림)
                jr      nz,fkc_no
                and     0E0h                    ; F1(비트5) F2(6) F3(7)
                cp      0E0h
                jr      z,fkc_no
                ld      b,0F1h
                bit     5,a
                jr      z,fkc_w
                inc     b
                bit     6,a
                jr      z,fkc_w
                inc     b
fkc_w:          push    bc
fkc_r:          ld      a,6
                ld      ix,SNSMAT
                call    bios
                and     0E0h
                cp      0E0h
                jr      nz,fkc_r
                ld      ix,KILBUF               ; 남은 글쇠 비움
                call    bios
                pop     bc
                ld      a,b
                ret
fkc_no:         xor     a
                ret
KILBUF          equ     0156h

typeclr:        xor     a
                ld      (typelen),a
                ret

; ======================================================================
; 타이핑 찾기: 글자를 붙이며 그 글자로 시작하는 항목으로 커서를 옮김. 못 찾으면 글자를 버림.
; ======================================================================
type_char:      ld      b,a
                ld      a,(typelen)
                cp      12
                ret     nc
                ld      e,a
                ld      d,0
                ld      hl,typebuf
                add     hl,de
                ld      a,b
                cp      'a'
                jr      c,tc_up
                cp      'z'+1
                jr      nc,tc_up
                and     0DFh                    ; 대문자로
tc_up:          ld      (hl),a
                ld      a,(typelen)
                inc     a
                ld      (typelen),a
                ld      hl,(JIFFY)
                ld      (type_t),hl
                call    cur_ix
                call    find_prefix
                ret     nc                      ; 찾음 (커서 이동)
                ld      a,(typelen)             ; 못 찾음: 글자 버림
                dec     a
                ld      (typelen),a
                ret

; IX = 패널. typebuf(typelen 글자)로 시작하는 항목을 찾아 커서를 옮김. 찾으면 carry
find_prefix:    ld      l,(ix+PS_COUNT)
                ld      h,(ix+PS_COUNT+1)
                ld      (fp_n),hl
                ld      hl,0
                ld      (fp_i),hl
fp_loop:        ld      hl,(fp_i)
                ld      de,(fp_n)
                or      a
                sbc     hl,de
                jr      nc,fp_none
                ld      hl,(fp_i)
                call    ptr_get                 ; HL = 항목
                call    search_name             ; nmbuf = 긴 이름 (없으면 8.3 이름) "NAME.EXT"
                ld      a,(typelen)
                ld      b,a
                ld      hl,typebuf
                ld      de,nmbuf
fp_cmp:         ld      a,(de)
                cp      (hl)
                jr      nz,fp_next
                inc     hl
                inc     de
                djnz    fp_cmp
                ld      hl,(fp_i)
                call    set_cur
                or      a                       ; carry 0
                ret
fp_next:        ld      hl,(fp_i)
                inc     hl
                ld      (fp_i),hl
                jr      fp_loop
fp_none:        scf
                ret

; IX = 패널. nmbuf 와 같은 이름(upname)을 가진 항목으로 커서. 찾으면 carry
find_exact:     ld      l,(ix+PS_COUNT)
                ld      h,(ix+PS_COUNT+1)
                ld      (fp_n),hl
                ld      hl,0
                ld      (fp_i),hl
fe_loop:        ld      hl,(fp_i)
                ld      de,(fp_n)
                or      a
                sbc     hl,de
                jr      nc,fp_none
                ld      hl,(fp_i)
                call    ptr_get
                call    disp_name
                ld      hl,upname
                ld      de,nmbuf
fe_cmp:         ld      a,(de)
                cp      (hl)
                jr      nz,fe_next
                or      a
                jr      z,fe_hit
                inc     hl
                inc     de
                jr      fe_cmp
fe_hit:         ld      hl,(fp_i)
                call    set_cur
                or      a
                ret
fe_next:        ld      hl,(fp_i)
                inc     hl
                ld      (fp_i),hl
                jr      fe_loop

; ======================================================================
; 커서 이동 (IX = 패널)
; ======================================================================
move_up:        ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                ld      a,h
                or      l
                ret     z
                dec     hl
                jr      set_cur
move_down:      ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                inc     hl
                ld      e,(ix+PS_COUNT)
                ld      d,(ix+PS_COUNT+1)
                or      a
                sbc     hl,de
                add     hl,de
                ret     nc                      ; 마지막이면 그대로
                jr      set_cur
page_up:        ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                ld      a,(v_lrows)
                dec     a
                ld      e,a
                ld      d,0
                or      a
                sbc     hl,de
                jr      nc,set_cur
                ld      hl,0
                jr      set_cur
page_down:      ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                ld      a,(v_lrows)
                dec     a
                ld      e,a
                ld      d,0
                add     hl,de
                ld      e,(ix+PS_COUNT)
                ld      d,(ix+PS_COUNT+1)
                ld      a,d
                or      e
                ret     z
                dec     de                      ; 마지막 번호
                or      a
                sbc     hl,de
                add     hl,de
                jr      c,set_cur
                ex      de,hl
                jr      set_cur
go_home:        ld      hl,0
; HL = 새 커서 번호. 화면 맨 위(top)를 맞춤
set_cur:        ld      (ix+PS_CUR),l
                ld      (ix+PS_CUR+1),h
                ld      e,(ix+PS_TOP)
                ld      d,(ix+PS_TOP+1)
                or      a
                sbc     hl,de
                jr      nc,sc_below
                add     hl,de                   ; 커서 < top 이면 top = 커서
                ld      (ix+PS_TOP),l
                ld      (ix+PS_TOP+1),h
                ret
sc_below:       ld      a,(v_lrows)
                ld      e,a
                ld      d,0
                or      a
                sbc     hl,de
                ret     c                      ; 커서 - top < 줄 수 이면 그대로
                ld      l,(ix+PS_CUR)           ; 커서 - top >= 20 이면 top = 커서 - 19
                ld      h,(ix+PS_CUR+1)
                ld      a,(v_lrows)
                dec     a
                ld      e,a
                ld      d,0
                or      a
                sbc     hl,de
                ld      (ix+PS_TOP),l
                ld      (ix+PS_TOP+1),h
                ret

; ======================================================================
; 패널 상태
; ======================================================================
cur_ix:         ld      a,(act)
panel_ix:       or      a
                ld      ix,pstate
                ret     z
                ld      ix,pstate+PSZ
                ret

panels_init:    ld      c,F_CURDRV
                call    BDOS
                ld      (cur_drive),a
                ld      ix,pstate
                ld      hl,PTR0
                ld      (ix+PS_PTR),l
                ld      (ix+PS_PTR+1),h
                ld      hl,ENT0
                ld      (ix+PS_ENT),l
                ld      (ix+PS_ENT+1),h
                ld      hl,LN0
                ld      (ix+PS_LNB),l
                ld      (ix+PS_LNB+1),h
                ld      ix,pstate+PSZ
                ld      hl,PTR1
                ld      (ix+PS_PTR),l
                ld      (ix+PS_PTR+1),h
                ld      hl,ENT1
                ld      (ix+PS_ENT),l
                ld      (ix+PS_ENT+1),h
                ld      hl,LN1
                ld      (ix+PS_LNB),l
                ld      (ix+PS_LNB+1),h
                ; 경로 = 현재 드라이브 + 현재 폴더
                ld      hl,pstate+PS_PATH
                ld      a,(cur_drive)
                add     a,'A'
                ld      (hl),a
                inc     hl
                ld      (hl),':'
                inc     hl
                ld      (hl),'\'
                inc     hl
                ld      (hl),0
                ; 현재 폴더: _GETCD 가 64 바이트를 채우므로 받아서 복사 (패널 칸 바로 앞의 번지)
                ld      de,oldpath
                ld      a,(cur_drive)
                inc     a
                ld      b,a
                ld      c,F_GETCD
                call    BDOS
                ld      hl,oldpath
                ld      de,pstate+PS_PATH+3
                ld      b,59
pi_cd:          ld      a,(hl)
                ld      (de),a
                or      a
                jr      z,pi_cd_e
                inc     hl
                inc     de
                djnz    pi_cd
                xor     a
                ld      (de),a
pi_cd_e:
                ld      hl,pstate+PS_PATH       ; 다른 패널과 같은 경로
                ld      de,pstate+PSZ+PS_PATH
                ld      bc,64
                ldir
                xor     a
                call    panel_ix
                call    panel_load
                ld      a,1
                call    panel_ix
                jp      panel_load

; IX = 패널: 경로 폴더를 읽고 정렬, 커서가 없으면 맨 위 0 으로
panel_load:     call    dir_read2               ; 빠른 길: 디렉터리 원본 섹터를 한 번만 읽어 항목·긴 이름을 같이 만듦
                jr      z,pl_sort
                call    dir_read                ; (안 되면) 옛 방식: DOS FFIRST/FNEXT + 긴 이름 따로
                ld      a,(dir_err)
                cp      E_NOFIL
                call    nz,explain_err          ; 읽기 오류는 알림 글 표시
                push    ix
                call    lfn_scan                ; 긴 이름 읽기 (거부 시 8.3 이름만)
                pop     ix
pl_sort:        call    dir_sort
                xor     a
                ld      (ix+PS_CUR),a
                ld      (ix+PS_CUR+1),a
                ld      (ix+PS_TOP),a
                ld      (ix+PS_TOP+1),a
                ret

; IX = 패널의 경로 = oldpath (실패하면 되돌리려고)
path_save:      push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      de,oldpath
                ld      bc,64
                ldir
                ret
path_restore:   push    ix
                pop     de
                ld      hl,PS_PATH
                add     hl,de
                ex      de,hl
                ld      hl,oldpath
                ld      bc,64
                ldir
                ret

; ======================================================================
; 디렉터리 읽기 (IX = 패널). dir_err = 마지막 오류 (E_NOFIL = 정상 끝)
; ======================================================================
dir_read:       ld      hl,0
                ld      (cnt_tmp),hl
                ld      (ix+PS_COUNT),l
                ld      (ix+PS_COUNT+1),h
                ld      (ix+PS_SEL),l           ; 고른 항목 수도 0 으로
                ld      (ix+PS_SEL+1),h
                ld      l,(ix+PS_LNB)           ; 긴 이름 저장소 비우기
                ld      h,(ix+PS_LNB+1)
                ld      (ix+PS_LNP),l
                ld      (ix+PS_LNP+1),h
                ; 찾을 이름 = 경로 + (끝이 \ 아니면 \) + *.*
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      de,specbuf
dr_cp:          ld      a,(hl)
                or      a
                jr      z,dr_cp_end
                ld      (de),a
                inc     hl
                inc     de
                jr      dr_cp
dr_cp_end:      dec     de
                ld      a,(de)
                inc     de
                cp      '\'
                jr      z,dr_star
                ld      a,'\'
                ld      (de),a
                inc     de
dr_star:        ld      hl,txt_star
                ld      bc,4
                ldir
                ld      de,specbuf
                ld      b,SRCHATTR
                ld      c,F_FFIRST
                call    bdos_fib
                jr      nz,dr_end
dr_loop:        call    dr_add
                ld      c,F_FNEXT
                call    bdos_fib
                jr      z,dr_loop
dr_end:         ld      (dir_err),a
                ld      hl,(cnt_tmp)            ; 항목 개수를 패널에
                ld      (ix+PS_COUNT),l
                ld      (ix+PS_COUNT+1),h
                ld      a,(dir_err)
                ret

; FIB 를 IX 와 함께 DOS 호출 (패널 IX 는 보존). 돌아오면 A = 오류 코드, Z = 성공
bdos_fib:       push    ix
                ld      ix,FIB
                call    BDOS
                pop     ix
                or      a
                ret

; FIB 한 항목을 IX 패널 목록에 추가
dr_add:         ld      a,(FIB+14)
                and     08h                     ; 볼륨 이름은 건너뜀
                ret     nz
                ld      hl,FIB+1
                ld      a,(hl)
                cp      '.'
                jr      nz,da_ok
                inc     hl
                ld      a,(hl)
                or      a
                ret     z                       ; "." 는 건너뜀
da_ok:          ld      hl,(cnt_tmp)
                ld      de,MAXE
                or      a
                sbc     hl,de
                ret     nc                      ; 가득 참  ; 항목 주소 = ENT + 개수 x 24
                ld      hl,(cnt_tmp)
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,de                   ; ×3
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; ×24
                ld      e,(ix+PS_ENT)
                ld      d,(ix+PS_ENT+1)
                add     hl,de
                ld      (en_p),hl
                ; 이름·확장자는 공백으로
                ld      d,h
                ld      e,l
                inc     de
                ld      (hl),' '
                ld      bc,10
                ldir
                ld      de,(en_p)
                ld      hl,FIB+1
                ld      a,(hl)
                cp      '.'
                jr      nz,da_name
                ld      (de),a                  ; ".." 항목: 이름 = ".."
                inc     de
                ld      (de),a
                jr      da_attr
da_name:        ld      b,8
da_n1:          ld      a,(hl)
                or      a
                jr      z,da_attr
                cp      '.'
                jr      z,da_dot
                ld      (de),a
                inc     hl
                inc     de
                djnz    da_n1
                ld      a,(hl)                  ; 기본 이름이 8글자를 채우면 다음 글자는 '.' 검사 없이 확장자로
                cp      '.'
                jr      nz,da_attr
da_dot:         inc     hl                      ; '.' 건너뜀
                ld      de,(en_p)
                ld      a,EN_EXT
                add     a,e
                ld      e,a
                jr      nc,da_e0
                inc     d
da_e0:          ld      b,3
da_e1:          ld      a,(hl)
                or      a
                jr      z,da_attr
                ld      (de),a
                inc     hl
                inc     de
                djnz    da_e1
da_attr:        ld      hl,(en_p)               ; 속성·시간·날짜(5바이트) 복사 (FIB +11)
                ld      de,EN_ATTR
                add     hl,de
                ex      de,hl
                ld      hl,FIB+14
                ld      bc,5
                ldir
                ld      hl,FIB+21               ; 크기(4바이트) 복사 (FIB +16)
                ld      bc,4
                ldir
                xor     a
                ld      (de),a                  ; 표시(+20) = 0
                inc     de
                ld      (de),a                  ; 긴 이름 번지(+21,+22) = 0 (없음)
                inc     de
                ld      (de),a
da_ptr:         ld      hl,(cnt_tmp)           ; 포인터 배열 [개수] = 항목 주소
                add     hl,hl
                ld      e,(ix+PS_PTR)
                ld      d,(ix+PS_PTR+1)
                add     hl,de
                ld      de,(en_p)
                ld      (hl),e
                inc     hl
                ld      (hl),d
                ld      hl,(cnt_tmp)
                inc     hl
                ld      (cnt_tmp),hl
                ret

; ======================================================================
; 정렬: 폴더 먼저, 같은 부류 안에서는 이름 순 (".." 가 맨 앞). 포인터 배열을 정렬
; ======================================================================
dir_sort:       ld      l,(ix+PS_COUNT)
                ld      h,(ix+PS_COUNT+1)
                ld      (sh_n),hl
                ld      a,h
                or      a
                jr      nz,ds_go
                ld      a,l
                cp      2
                ret     c                       ; 항목이 2개 미만이면 정렬할 것 없음
ds_go:          ld      e,(ix+PS_PTR)
                ld      d,(ix+PS_PTR+1)
                ld      (sh_base),de
                ld      hl,gaps
                ld      (sh_gp),hl
ds_gap:         ld      hl,(sh_gp)
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                inc     hl
                ld      (sh_gp),hl
                ld      a,d
                or      e
                ret     z                       ; 개수 0 이면 끝
                ld      (sh_h),de
                ld      hl,(sh_n)
                or      a
                sbc     hl,de
                jr      c,ds_gap                ; 간격이 개수보다 크면 건너뜀
                jr      z,ds_gap                ; 같아도 건너뜀
                ld      hl,(sh_h)
                ld      (sh_i),hl
ds_i:           ld      hl,(sh_i)
                ld      de,(sh_n)
                or      a
                sbc     hl,de
                jr      nc,ds_gap               ; i >= n 이면 이 간격은 끝
                ld      hl,(sh_i)
                call    sh_get
                ld      (sh_tmp),de
                ld      hl,(sh_i)
                ld      (sh_j),hl
ds_j:           ld      hl,(sh_j)
                ld      de,(sh_h)
                or      a
                sbc     hl,de
                jr      c,ds_put                ; j < h
                ld      (sh_k),hl               ; k = j - h
                call    sh_get                  ; DE = a[k]
                ld      hl,(sh_tmp)
                call    ent_cmp                 ; tmp < a[k] ?
                jr      nc,ds_put
                ld      hl,(sh_k)
                call    sh_get
                ld      hl,(sh_j)
                call    sh_put                  ; a[j] = a[k]
                ld      hl,(sh_k)
                ld      (sh_j),hl
                jr      ds_j
ds_put:         ld      hl,(sh_j)
                ld      de,(sh_tmp)
                call    sh_put
                ld      hl,(sh_i)
                inc     hl
                ld      (sh_i),hl
                jr      ds_i

sh_get:         add     hl,hl                   ; HL = 번호 → DE = 배열[번호]
                ld      de,(sh_base)
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ret
sh_put:         add     hl,hl                   ; 배열[HL] = DE
                ld      bc,(sh_base)
                add     hl,bc
                ld      (hl),e
                inc     hl
                ld      (hl),d
                ret

; HL = 항목 A, DE = 항목 B → carry 1 이면 A 가 앞
ent_cmp:        ld      (cmp_a),hl
                ld      (cmp_b),de
                call    ent_class
                ld      b,a
                ld      hl,(cmp_b)
                call    ent_class
                ld      c,a
                ld      a,b
                cp      c
                ret     nz                      ; 부류가 다르면 그 결과로 정함 (carry = b < c)
                ld      a,(sort_mode)           ; 정렬 방식: 0 이름, 1 확장자, 2 크기(큰 것 먼저), 3 날짜(새 것 먼저)
                or      a
                jr      z,ec_name
                cp      1
                jr      z,ec_ext
                cp      2
                jr      z,ec_size
                ld      hl,(cmp_a)
                ld      de,(cmp_b)
                ld      b,2
                ld      a,EN_DATE+1
                call    cmp_le
                ret     nz
                ld      hl,(cmp_a)
                ld      de,(cmp_b)
                ld      b,2
                ld      a,EN_TIME+1
                call    cmp_le
                ret     nz
                jr      ec_name
ec_size:        ld      hl,(cmp_a)
                ld      de,(cmp_b)
                ld      b,4
                ld      a,EN_SIZE+3
                call    cmp_le
                ret     nz
                jr      ec_name
ec_ext:         ld      hl,(cmp_a)
                ld      de,(cmp_b)
                ld      bc,EN_EXT
                add     hl,bc
                ex      de,hl
                add     hl,bc
                ex      de,hl
                ld      b,3
ece_l:          ld      a,(de)
                ld      c,a
                ld      a,(hl)
                cp      c
                ret     nz
                inc     hl
                inc     de
                djnz    ece_l
ec_name:        ld      hl,(cmp_a)
                ld      de,(cmp_b)
                ld      b,11
ec_l:           ld      a,(de)
                ld      c,a
                ld      a,(hl)
                cp      c
                ret     nz
                inc     hl
                inc     de
                djnz    ec_l
                or      a                       ; 같음 → carry 0
                ret

; 큰 수 비교 (낮은 바이트가 먼저인 값): HL = A 항목, DE = B 항목, A = 가장 높은 바이트 위치, B = 바이트 수
; 같으면 Z, 다르면 NZ 이고 A 가 더 크면 C (= A 를 앞에 둠)
cmp_le:         ld      c,a
                push    bc
                ld      b,0
                add     hl,bc
                ex      de,hl
                add     hl,bc
                ex      de,hl
                pop     bc
cl_l:           ld      a,(de)
                cp      (hl)
                ret     nz
                dec     hl
                dec     de
                djnz    cl_l
                xor     a
                ret
; HL = 항목 → A = 부류 (0 "..", 1 폴더, 2 파일)
ent_class:      ld      a,(hl)
                cp      '.'
                jr      nz,ecl_1
                inc     hl
                ld      a,(hl)
                dec     hl
                cp      '.'
                jr      nz,ecl_1
                xor     a
                ret
ecl_1:          ld      de,EN_ATTR
                add     hl,de
                ld      a,(hl)
                and     10h
                ld      a,2
                ret     z
                ld      a,1
                ret

; ======================================================================
; 폴더 들어가기 / 올라가기 / 드라이브 바꾸기
; ======================================================================
; IX = 패널
panel_enter:    ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                ret     z
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                ld      (en_p),hl
                ld      de,EN_ATTR
                add     hl,de
                ld      a,(hl)
                and     10h
                jp      z,run_cur                      ; 파일: A 단계에서는 아무것도 안 함
                ld      hl,(en_p)
                call    disp_name
                ld      a,(nmbuf)
                cp      '.'
                jp      z,panel_up              ; ".." 이면 위로
                call    path_save
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      b,0                     ; 경로 길이
pe_len:         ld      a,(hl)
                or      a
                jr      z,pe_len_e
                inc     hl
                inc     b
                jr      pe_len
pe_len_e:       dec     hl
                ld      a,(hl)
                inc     hl
                cp      '\'
                jr      z,pe_noslash
                ld      (hl),'\'
                inc     hl
                inc     b
pe_noslash:     ld      a,b
                cp      40
                jr      nc,pe_toolong           ; 너무 길면 거절 (이름 12글자 + 여유 한 글자)
                ld      de,nmbuf
pe_app:         ld      a,(de)
                ld      (hl),a
                or      a
                jr      z,pe_loaded
                inc     hl
                inc     de
                jr      pe_app
pe_loaded:      call    panel_load
                ld      a,(dir_err)
                cp      E_NOFIL
                ret     z                       ; 정상
                call    explain_err
                call    path_restore
                jp      panel_load
pe_toolong:     call    path_restore
                ld      hl,txt_toolong
                ld      (msg_p),hl
                ret

; IX = 패널: 상위 폴더로 올라가서 방금 있던 폴더 이름에 커서를 맞춤
panel_up:       push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      (up_p),hl
                ld      d,h
                ld      e,l                     ; DE = 경로 시작
                inc     de
                inc     de                      ; DE = 루트 '\' 자리 ("A:\" 의 세 번째 글자)
                ld      (up_root),de
                ld      b,h
                ld      c,l                     ; (BC 는 쓰지 않음)
                ld      hl,(up_p)
                ld      (up_last),de            ; 마지막 '\' 자리 (처음에는 루트)
pu_scan:        ld      a,(hl)
                or      a
                jr      z,pu_scan_e
                cp      '\'
                jr      nz,pu_sn
                ld      (up_last),hl
pu_sn:          inc     hl
                jr      pu_scan
pu_scan_e:      ld      hl,(up_last)
                ld      de,(up_root)
                or      a
                sbc     hl,de
                ld      a,h
                or      l
                jr      nz,pu_do
                ld      hl,(up_root)            ; 마지막 '\' 가 루트
                inc     hl
                ld      a,(hl)
                or      a
                ret     z                       ; 이미 루트 ("A:\")
pu_do:          call    path_save
                ld      hl,(up_last)
                inc     hl                      ; 이어지는 폴더 이름
                ld      de,upname
pu_nm:          ld      a,(hl)
                ld      (de),a
                inc     hl
                inc     de
                or      a
                jr      nz,pu_nm
                ld      hl,(up_last)
                ld      de,(up_root)
                or      a
                sbc     hl,de
                ld      hl,(up_last)
                jr      nz,pu_cut
                inc     hl                      ; 루트이면 "A:\" 까지만
pu_cut:         ld      (hl),0
                call    panel_load
                ld      a,(dir_err)
                cp      E_NOFIL
                jr      z,pu_ok
                call    explain_err
                call    path_restore
                jp      panel_load
pu_ok:          jp      find_exact              ; 올라온 상위 폴더에서 방금 있던 폴더 이름에 커서

drive_prompt:   ld      a,5                     ; (0.12.15) F9 드라이브 풀다운은 오버레이 5
                ld      b,2
                jp      ov_go

drm_n:          db      0
drm_sel:        db      0
drm_i:          db      0
drv_list:       ds      18
drv_p:          dw      0

; 오류 코드 A 에 해당하는 알림 글 (HL = 글)
explain_err:    ld      b,a
                ld      de,errbuf
                ld      c,F_EXPLAIN
                call    BDOS
                ld      hl,errbuf
                ld      (msg_p),hl
                ret

; ======================================================================
; 긴 이름 (VFAT) 읽기
; ======================================================================
; ======================================================================
; 긴 이름 (VFAT). DOS2·Nextor 의 파일 호출은 긴 이름을 주지 않으므로 디렉터리 섹터를 직접 읽어 해석한다.
; - 부트 섹터(BPB)로 FAT·루트·데이터 위치를 알아내고, 루트는 고정 구간, 하위 폴더는 클러스터 사슬을 따라간다.
; - 속성 0Fh 항목(긴 이름 조각)을 모아, 뒤따르는 8.3 항목의 검사합과 맞으면 그 8.3 항목의 긴 이름으로 삼는다.
; - 제한: 섹터 512 바이트, 섹터 번호 16 비트(32MB 이하 볼륨), FAT12/16 만 지원.
; - 글자: UCS-2 중 U+0000~U+00FF (Latin-1) 만 보이고, 나머지는 '?' 로 바꿈.
; ======================================================================
; 원본 섹터 읽기: HL = 섹터 번호(16 비트), B = 개수, DE = 받을 곳, lf_drv = 드라이브
rd_abs:         ld      (ra_s),hl
                ld      a,b
                ld      (ra_n),a
                push    ix
                ld      c,F_SETDTA              ; DE = 받을 곳
                call    BDOS
                ld      a,(ra_f)
                ld      hl,(ra_hi)
                or      h
                or      l
                jr      nz,ra_drv               ; 윗 16 비트가 0 이 아니면 (또는 강제) Nextor _RDDRV
                ld      de,(ra_s)
                ld      a,(ra_n)
                ld      h,a
                ld      a,(lf_drv)
                ld      l,a
                ld      c,F_RDABS
                call    BDOS
                pop     ix
                or      a
                ret
ra_drv:         ld      a,(lf_ord)              ; 0 = HL:DE = 윗:아랫,  1 = 아랫:윗
                ld      hl,(ra_hi)
                ld      de,(ra_s)
                or      a
                jr      z,ra_o
                ex      de,hl
ra_o:           ld      a,(ra_n)
                ld      b,a
                ld      a,(lf_drv)
                ld      c,F_RDDRV
                call    BDOS
                pop     ix
                or      a
                ret

; HL = 섹터 번호 → SECBUF 에 읽은 섹터
rd_sec:         push    hl
                ld      hl,(lf_sh)              ; 디렉터리 섹터: 윗 16 비트는 lf_sh
                ld      (ra_hi),hl
                pop     hl
                ld      b,1
                ld      de,SECBUF
                jr      rd_abs

; IX = 패널: 긴 이름을 읽어 항목에 붙임
lfn_scan:       ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                ret     z
                ld      (lf_panel),ix
                ld      l,(ix+PS_LNB)
                ld      h,(ix+PS_LNB+1)
                ld      de,LNSZ
                add     hl,de
                ld      (lf_end),hl             ; 저장소 끝 주소
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                xor     a
                ld      (lf_mode),a             ; 0 = 읽어서 항목에 붙임
                ld      de,lf_proc
                ld      (lf_hook),de
                call    lf_geom
                ret     nz
                jp      lf_walk

; HL = 폴더 경로 ("X:\...") → 드라이브·BPB 에서 FAT·루트·데이터 위치를 계산.
lf_geom:        ld      (lf_path),hl
                ld      a,(hl)
                sub     'A'
                ld      (lf_drv),a
                xor     a
                ld      (lf_valid),a
                ld      (lf_done),a
                ld      hl,0FFFFh
                ld      (lf_fcache),hl
                ld      hl,0
                ld      (lf_sh),hl
                xor     a
                ld      (lf_big),a
                ld      (ra_f),a
                ; --- 부트 섹터 (BPB) ---
                ld      hl,0
                call    rd_sec
                ret     nz
                ld      a,(SECBUF+0Bh)
                or      a
                ret     nz
                ld      a,(SECBUF+0Ch)
                cp      2
                ret     nz                      ; 섹터 512 바이트만 지원
                ld      a,(SECBUF+0Dh)          ; 클러스터당 섹터 수 (1,2,4,...)
                or      a
                jp      z,lg_fail
                ld      (lf_spc),a
                ld      b,0
lfs_sh:         srl     a
                jr      z,lfs_she
                inc     b
                jr      lfs_sh
lfs_she:        ld      a,b
                ld      (lf_shift),a
                ld      hl,(SECBUF+0Eh)         ; 예약 섹터 = FAT 시작
                ld      (lf_fat),hl
                ld      de,(SECBUF+16h)         ; FAT 하나의 섹터 수
                ld      a,d
                or      e
                jp      z,lg_fail               ; 0 = FAT32 (Nextor 는 FAT12/16 만 씀)
                ld      a,(SECBUF+10h)          ; FAT 개수
                or      a
                jp      z,lg_fail
                ld      b,a
lfs_fa:         add     hl,de
                djnz    lfs_fa
                ld      (lf_root),hl            ; 루트 디렉터리 시작
                ld      hl,(SECBUF+11h)         ; 루트 항목 수 → 섹터 수 (÷16)
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                ld      (lf_rsecs),hl
                ld      de,(lf_root)
                add     hl,de
                ld      (lf_data),hl            ; 데이터 시작
                ld      hl,(SECBUF+13h)         ; 전체 섹터 (0 이면 큰 볼륨: 처리 안 함)
                ld      a,h
                or      l
                jp      z,lg_big                ; 0 이면 32 비트 전체 섹터 수 (32MB 넘는 볼륨)
                ld      de,(lf_data)
                or      a
                sbc     hl,de
                jp      c,lg_fail
                ld      a,(lf_shift)
                or      a
                jr      z,lfs_cl
lfs_cs:         srl     h
                rr      l
                dec     a
                jr      nz,lfs_cs
lfs_cl:         inc     hl
                inc     hl
                ld      (lf_clmax),hl           ; 클러스터 번호 상한 (이 값 미만이 유효)
                ld      de,4087                 ; 클러스터 수가 4085 이상이면 FAT16
                or      a
                sbc     hl,de
                ld      a,0
                jr      c,lfs_f12
                inc     a
lfs_f12:        ld      (lf_fat16),a
                ld      a,(lf_big)
                or      a
                jr      nz,lg_ord               ; 큰 볼륨이면 32 비트 읽기 방식 확인
                xor     a                       ; 성공: Z
                ret
lg_fail:        or      1                       ; 실패: NZ
                ret
; 큰 볼륨: 전체 섹터 수(32 비트, BPB+20h) - 데이터 시작 = 데이터 섹터 수 → ÷ 섹터/클러스터 = 클러스터 수 (FAT16 이라 65535 이하)
lg_big:         ld      a,1
                ld      (lf_big),a
                ld      hl,(SECBUF+20h)
                ld      de,(SECBUF+22h)
                ld      bc,(lf_data)
                or      a
                sbc     hl,bc
                jr      nc,lgb_1
                dec     de
lgb_1:          bit     7,d
                jp      nz,lg_fail
                ld      a,(lf_shift)
                or      a
                jr      z,lgb_c
lgb_s:          srl     d
                rr      e
                rr      h
                rr      l
                dec     a
                jr      nz,lgb_s
lgb_c:          ld      a,d
                or      e
                jp      z,lfs_cl
                ld      hl,0FFF0h               ; 너무 크면 FAT16 최대로
                jp      lfs_cl
; Nextor _RDDRV 가 쓸 수 있는지, HL:DE 순서가 어느 쪽인지 확인: FAT 첫 섹터를 _RDABS 로 읽은 것과 _RDDRV 로 읽은 것을 비교
lg_ord:         ld      hl,(lf_fat)
                ld      b,1
                ld      de,FATBUF
                xor     a
                ld      (ra_f),a
                ld      (ra_hi),a
                ld      (ra_hi+1),a
                call    rd_abs
                ret     nz
                ld      a,1
                ld      (ra_f),a                ; _RDDRV 강제
                xor     a
                ld      (lf_ord),a
                call    lo_try
                jr      z,lo_ok
                ld      a,1
                ld      (lf_ord),a
                call    lo_try
                jr      z,lo_ok
                xor     a
                ld      (ra_f),a
                jr      lg_fail                 ; _RDDRV 가 안 됨 → 큰 볼륨은 긴 이름 못 읽음
lo_ok:          xor     a
                ld      (ra_f),a
                ret
lo_try:         ld      hl,(lf_fat)
                ld      b,1
                ld      de,SECBUF
                call    rd_abs
                jr      nz,lot_no
                ld      hl,SECBUF
                ld      de,FATBUF
                ld      bc,512
lot_c:          ld      a,(de)
                cpi
                jr      nz,lot_no
                inc     de
                jp      pe,lot_c
                xor     a                       ; 같음: Z
                ret
lot_no:         or      1
                ret

; 폴더(lf_path)의 디렉터리 섹터를 차례로 읽으며 섹터마다 lf_hook 을 부른다 (hook 은 아래).
lf_walk:        ld      hl,(lf_path)            ; --- 폴더가 루트인가 ("X:\") ---
                inc     hl
                inc     hl
                inc     hl
                ld      a,(hl)
                or      a
                jp      z,lfs_root
                ; 하위 폴더: 상위 폴더 전체에서 이 폴더 항목을 찾아 시작 클러스터를 얻음
                ld      hl,(lf_path)
                ld      de,specbuf
lfs_cp:         ld      a,(hl)
                ld      (de),a
                inc     hl
                inc     de
                or      a
                jr      nz,lfs_cp
                ld      de,specbuf
                ld      b,SRCHATTR
                ld      c,F_FFIRST
                call    bdos_fib
                ret     nz
                ld      hl,(FIB+19)             ; 시작 클러스터
                ld      (lf_cl),hl
lfs_cll:        ld      hl,(lf_cl)              ; --- 클러스터 사슬 ---
                ld      de,2
                or      a
                sbc     hl,de
                ret     c
                ld      hl,(lf_cl)
                ld      de,(lf_clmax)
                or      a
                sbc     hl,de
                ret     nc
                ld      hl,(lf_cl)
                dec     hl
                dec     hl
                ld      de,0                    ; DE:HL = (클러스터-2) × 섹터/클러스터 (32 비트)
                ld      a,(lf_shift)
                or      a
                jr      z,lfs_ns
lfs_sl:         add     hl,hl
                rl      e
                rl      d
                dec     a
                jr      nz,lfs_sl
lfs_ns:         push    de
                ld      de,(lf_data)
                add     hl,de
                pop     de
                jr      nc,lfs_nc
                inc     de
lfs_nc:         ld      (lf_s),hl
                ld      (lf_sh),de
                ld      a,(lf_spc)
                ld      (lf_n8),a
lfs_cs2:        ld      hl,(lf_s)
                call    rd_sec
                ret     nz
                call    lf_dispatch
                ld      a,(lf_done)
                or      a
                ret     nz
                ld      hl,(lf_s)
                inc     hl
                ld      (lf_s),hl
                ld      a,h
                or      l
                jr      nz,lfs_ni
                ld      hl,(lf_sh)              ; 아랫 16 비트가 넘침 → 윗 16 비트 +1
                inc     hl
                ld      (lf_sh),hl
lfs_ni:         ld      hl,lf_n8
                dec     (hl)
                jr      nz,lfs_cs2
                ld      hl,(lf_cl)
                call    fat_next
                ret     c
                ld      (lf_cl),hl
                jr      lfs_cll
lfs_root:       ld      hl,0
                ld      (lf_sh),hl
                ld      hl,(lf_root)            ; --- 루트: 고정 구간 ---
                ld      (lf_s),hl
                ld      hl,(lf_rsecs)
                ld      (lf_n),hl
lfs_rl:         ld      hl,(lf_n)
                ld      a,h
                or      l
                ret     z
                ld      hl,(lf_s)
                call    rd_sec
                ret     nz
                call    lf_dispatch
                ld      a,(lf_done)
                or      a
                ret     nz
                ld      hl,(lf_s)
                inc     hl
                ld      (lf_s),hl
                ld      hl,(lf_n)
                dec     hl
                ld      (lf_n),hl
                jr      lfs_rl

; HL = 클러스터 → HL = 다음 클러스터. 읽기 실패면 carry
fat_next:       ld      (fn_cl),hl
                ld      a,(lf_fat16)
                or      a
                jr      nz,fn_16
                ld      d,h                     ; FAT12: 바이트 위치 = 클러스터 × 3 / 2
                ld      e,l
                srl     d
                rr      e
                add     hl,de
                jr      fn_get
fn_16:          ld      e,h                     ; FAT16: 섹터 번호 = 클러스터 ÷ 256, 섹터 안 위치 = (클러스터 mod 256) × 2
                ld      d,0                     ; (클러스터 32768 이상도 16 비트가 안 넘치게)
                ld      h,0
                add     hl,hl
                ld      (fn_off),hl
                jr      fn_s
fn_get:         ld      (fn_off),hl
                ld      a,h
                srl     a                       ; 섹터 번호(FAT 안) = 위치 / 512
                ld      e,a
                ld      d,0
fn_s:           ld      hl,(lf_fcache)
                or      a
                sbc     hl,de
                jr      z,fn_have               ; 이미 읽어 둔 섹터
                ld      hl,(lf_fat)
                add     hl,de
                push    de
                push    hl
                ld      hl,0
                ld      (ra_hi),hl              ; FAT 는 앞쪽 (16 비트 섹터 번호)
                pop     hl
                ld      b,2                     ; 두 섹터 (FAT12 가 섹터 경계에 걸리는 경우)
                ld      de,FATBUF
                call    rd_abs
                pop     de
                scf
                ret     nz
                ld      (lf_fcache),de
fn_have:        ld      hl,(fn_off)
                ld      a,h
                and     01h
                ld      h,a                     ; 섹터 안 위치 (0~511)
                ld      de,FATBUF
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)                  ; DE = 항목 16 비트
                ld      a,(lf_fat16)
                or      a
                jr      nz,fn_done
                ld      a,(fn_cl)               ; FAT12: 홀수면 위 12비트, 짝수면 아래 12비트
                and     1
                jr      z,fn_even
                srl     d
                rr      e
                srl     d
                rr      e
                srl     d
                rr      e
                srl     d
                rr      e
                jr      fn_done
fn_even:        ld      a,d
                and     0Fh
                ld      d,a
fn_done:        ex      de,hl
                or      a
                ret

lf_dispatch:    ld      hl,(lf_hook)
                jp      (hl)

; SECBUF 의 항목 16개를 처리 (읽기 모드 / 원본 조각 모으기 모드)
lf_proc:        ld      hl,SECBUF
                ld      b,16
lp_l:           push    bc
                push    hl
                call    lf_entry
                pop     hl
                pop     bc
                ld      a,(lf_done)
                or      a
                ret     nz
                ld      de,32
                add     hl,de
                djnz    lp_l
                ret

; HL = 32 바이트 항목
lf_entry:       ld      a,(hl)
                or      a
                jr      nz,le_1
                ld      a,1                     ; 0 = 읽기 모드
                ld      (lf_done),a
                ret
le_1:           cp      0E5h
                jr      z,le_inv                ; 지워진 항목
                push    hl
                ld      de,11
                add     hl,de
                ld      a,(hl)                  ; 속성
                pop     hl
                cp      0Fh
                jp      z,lf_piece
                and     08h
                jr      nz,le_inv               ; 볼륨 이름
                ld      a,(lf_mode)             ; 모드 2 = 항목을 바로 만듦
                cp      2
                jp      z,lf_mk2
                ld      a,(lf_valid)            ; 8.3 항목: 앞의 긴 이름 조각이 맞는가
                or      a
                ret     z
                ld      a,(lf_exp)
                cp      1
                jr      nz,le_inv
                call    lf_sum
                ld      b,a
                ld      a,(lf_chk)
                cp      b
                jr      nz,le_inv
                ld      a,(lf_mode)
                or      a
                jr      z,le_att
                ld      de,(lf_tgt)             ; 조각 모으기 모드: 찾는 이름이면 모음
                ld      b,11
le_cmp:         ld      a,(de)
                cp      (hl)
                jr      nz,le_inv
                inc     de
                inc     hl
                djnz    le_cmp
                ld      a,1
                ld      (lf_found),a
                ld      (lf_done),a
                ret
le_att:         call    lf_attach
le_inv:         xor     a
                ld      (lf_valid),a
                ret

; HL = 8.3 이름(11) → A = 검사합
lf_sum:         push    hl
                ld      b,11
                xor     a
ls_l:           rrca
                add     a,(hl)
                inc     hl
                djnz    ls_l
                pop     hl
                ret

; HL = 긴 이름 조각 항목
lf_piece:       ld      a,(hl)
                ld      c,a
                and     3Fh
                jr      z,le_inv
                cp      21
                jr      nc,le_inv
                ld      (lf_seq),a
                ld      a,c
                and     40h
                jr      z,lpc_mid
                ld      a,(lf_seq)              ; 마지막 조각(디렉터리에서는 맨 앞): 여기서 시작
                ld      (lf_exp),a
                xor     a
                ld      (lf_rawn),a
                push    hl
                ld      de,13
                add     hl,de
                ld      a,(hl)
                ld      (lf_chk),a
                ld      hl,LFNBUF
                ld      de,LFNBUF+1
                ld      (hl),0
                ld      bc,LFNBUF_SZ-1
                ldir
                pop     hl
                ld      a,1
                ld      (lf_valid),a
                jr      lpc_store
lpc_mid:        ld      a,(lf_valid)
                or      a
                ret     z
                ld      a,(lf_exp)
                dec     a
                ld      b,a
                ld      a,(lf_seq)
                cp      b
                jr      nz,le_inv               ; 순서가 맞지 않음
                push    hl
                ld      de,13
                add     hl,de
                ld      a,(lf_chk)
                cp      (hl)
                pop     hl
                jr      nz,le_inv               ; 검사합이 다름
lpc_store:      ld      a,(lf_seq)
                ld      (lf_exp),a
                push    hl
                call    lf_raw_add
                pop     hl
                push    hl
                pop     ix
                ld      a,(lf_seq)
                ld      hl,LFNBUF
                dec     a
                jr      z,lpc_d0
                ld      de,26
lpc_da:         add     hl,de
                dec     a
                jr      nz,lpc_da
lpc_d0:         ex      de,hl
                ld      hl,lfn_off
                ld      a,13
                ld      (lf_cnt),a
lpc_l:          ld      c,(hl)
                inc     hl
                push    hl
                push    ix
                pop     hl
                ld      b,0
                add     hl,bc
                ld      c,(hl)
                inc     hl
                ld      b,(hl)
                ld      a,b
                and     c
                inc     a
                jr      nz,lpc_st
                ld      b,a
                ld      c,a
lpc_st:         ld      a,c
                ld      (de),a
                inc     de
                ld      a,b
                ld      (de),a
                inc     de
                pop     hl
                ld      a,(lf_cnt)
                dec     a
                ld      (lf_cnt),a
                jr      nz,lpc_l
                ret

; HL = 8.3 이름(11): 패널에서 같은 이름의 항목을 찾아 LFNBUF 의 긴 이름을 붙임
lf_attach:      ld      (lf_sn),hl
                ld      ix,(lf_panel)
                ld      l,(ix+PS_ENT)
                ld      h,(ix+PS_ENT+1)
                ld      c,(ix+PS_COUNT)
                ld      b,(ix+PS_COUNT+1)
la_l:           ld      a,b
                or      c
                ret     z
                push    bc
                push    hl
                ld      de,(lf_sn)
                ld      b,11
la_c:           ld      a,(de)
                cp      (hl)
                jr      nz,la_next
                inc     de
                inc     hl
                djnz    la_c
                pop     hl                      ; 같은 이름: HL = 항목
                pop     bc
                jr      la_found
la_next:        pop     hl
                pop     bc
                ld      de,ENSZ
                add     hl,de
                dec     bc
                jr      la_l
la_found:       ld      (lf_en),hl
                call    lf_conv
                ld      hl,LFNBUF               ; 길이 (LFN_MAX 까지)
                ld      b,0
la_len:         ld      a,(hl)
                or      a
                jr      z,la_len_e
                ld      a,b
                cp      LFN_MAX
                jr      nc,la_len_e
                inc     hl
                inc     b
                jr      la_len
la_len_e:       ld      a,b
                or      a
                ret     z
                ld      (la_n),a
                ld      l,(ix+PS_LNP)
                ld      h,(ix+PS_LNP+1)
                ld      (la_dest),hl
                ld      e,a
                ld      d,0
                add     hl,de
                inc     hl                      ; 다음 저장 자리 = 현재 자리 + 길이 + 1
                ld      de,(lf_end)
                or      a
                sbc     hl,de
                ret     nc                      ; 저장소 주소가 가득 참
                add     hl,de
                ld      (ix+PS_LNP),l
                ld      (ix+PS_LNP+1),h
                ld      hl,LFNBUF
                ld      de,(la_dest)
                ld      a,(la_n)
                ld      c,a
                ld      b,0
                ldir
                xor     a
                ld      (de),a
                ld      hl,(lf_en)
                ld      de,EN_LFN
                add     hl,de
                ld      de,(la_dest)
                ld      (hl),e
                inc     hl
                ld      (hl),d
                ret

; A = Latin-1 글자 → A = 화면에 보일 글자. MSX 글꼴의 80h 이상 모양은 기종마다 달라서,
; 액센트 글자는 기본 글자로 바꿔 보임. 0 은 그대로 둠.
; ---- 빠른 디렉터리 읽기 (lf_mode = 2): 디렉터리 원본 섹터의 8.3 항목마다 패널 항목을 만들고, 바로 앞의
;      긴 이름 조각이 맞으면 그 이름을 붙임.  DOS FFIRST/FNEXT 를 건너뛰어 훨씬 빠름 (안 되면 옛 방식으로 되돌아감)
dir_read2:      push    ix
                call    d2_main
                pop     ix
                ret
d2_main:        ld      (lf_panel),ix
                ld      hl,0
                ld      (cnt_tmp),hl
                ld      (ix+PS_COUNT),l
                ld      (ix+PS_COUNT+1),h
                ld      (ix+PS_SEL),l
                ld      (ix+PS_SEL+1),h
                ld      l,(ix+PS_LNB)
                ld      h,(ix+PS_LNB+1)
                ld      (ix+PS_LNP),l
                ld      (ix+PS_LNP+1),h
                ld      de,LNSZ
                add     hl,de
                ld      (lf_end),hl
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                call    lf_geom
                ret     nz                      ; 못 읽는 볼륨·오류 → 옛 방식
                ld      a,2
                ld      (lf_mode),a
                ld      de,lf_proc
                ld      (lf_hook),de
                call    lf_walk
                xor     a
                ld      (lf_mode),a
                ld      hl,(cnt_tmp)
                ld      a,h
                or      l
                jr      z,d2_fail               ; 하나도 못 만들면 옛 방식으로 (오류 글도 거기서)
                ld      ix,(lf_panel)
                ld      (ix+PS_COUNT),l
                ld      (ix+PS_COUNT+1),h
                ld      a,E_NOFIL
                ld      (dir_err),a
                xor     a
                ret                             ; Z = 성공
d2_fail:        or      1
                ret
; HL = 원본 32바이트 항목 → 패널 항목 하나 만듦 (en_p = 그 주소). 건너뛰거나 가득 차면 C
mk_entry:       ld      (me_src),hl
                ld      a,(hl)
                cp      '.'
                jr      nz,me_ok
                inc     hl
                ld      a,(hl)
                cp      ' '
                scf
                ret     z                       ; "." 는 건너뜀
me_ok:          ld      hl,(cnt_tmp)
                ld      de,MAXE
                or      a
                sbc     hl,de
                jr      c,me_go
                scf
                ret                             ; 가득 참
me_go:          ld      hl,(cnt_tmp)
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,de
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; x 24
                ld      ix,(lf_panel)
                ld      e,(ix+PS_ENT)
                ld      d,(ix+PS_ENT+1)
                add     hl,de
                ld      (en_p),hl
                ex      de,hl                   ; DE = 항목 자리
                ld      hl,(me_src)
                ld      bc,12                   ; 이름 8 + 확장자 3 + 속성 1
                ldir
                ld      hl,(me_src)
                ld      bc,22
                add     hl,bc
                ld      bc,4                    ; 시간 2 + 날짜 2
                ldir
                ld      hl,(me_src)
                ld      bc,28
                add     hl,bc
                ld      bc,4                    ; 크기 4
                ldir
                xor     a
                ld      (de),a                  ; 표시(+20) = 0
                inc     de
                ld      (de),a                  ; 긴 이름 번지(+21,+22) = 0
                inc     de
                ld      (de),a
                call    da_ptr                  ; 포인터 배열 · 개수 올림
                or      a
                ret
; 모드 2: HL = 8.3 항목
lf_mk2:         push    hl
                call    mk_entry
                pop     hl
                jp      c,le_inv
                ld      a,(lf_valid)
                or      a
                ret     z
                ld      a,(lf_exp)
                cp      1
                jp      nz,le_inv
                call    lf_sum
                ld      b,a
                ld      a,(lf_chk)
                cp      b
                jp      nz,le_inv
                ld      hl,(en_p)
                ld      ix,(lf_panel)
                call    la_found                ; 방금 만든 항목에 긴 이름 붙임
                jp      le_inv
me_src:         dw      0
l1_map:         or      a
                ret     z
                cp      20h
                jr      c,l1_q
                cp      7Fh
                ret     c                       ; 20h~7Eh 는 그대로
                cp      0A0h
                jr      c,l1_q                  ; 7Fh~9Fh
                push    hl
                push    bc
                sub     0A0h
                ld      l,a
                ld      h,0
                ld      bc,l1_tab
                add     hl,bc
                ld      a,(hl)
                pop     bc
                pop     hl
                ret
l1_q:           ld      a,'?'
                ret
; A0h~FFh (96 글자)
l1_tab:         db      " !coLoY|S",22h,"Ca<--R-"       ; A0~AF
                db      "o+23",27h,"uP.,1o>???"         ; B0~BF
                db      "AAAAAAACEEEEIIII"              ; C0~CF
                db      "DNOOOOOxOUUUUYPs"              ; D0~DF
                db      "aaaaaaaceeeeiiii"              ; E0~EF
                db      "dnooooo/ouuuuypy"              ; F0~FF

; ======================================================================
; 긴 이름 복사. DOS 호출은 8.3 만 복사하므로 디렉터리 섹터를 직접 고친다.
; lf_capture : 원본 폴더에서 8.3 항목에 달린 긴 이름 조각을 모아 둠 (원본은 그대로)
; lf_install : 대상 폴더에서 방금 만든 8.3 항목을 찾아, 조각들 + 8.3 항목을 다시 씀
; lf_erase   : 지워진 8.3 항목에 달린 긴 이름 조각을 지움 (E5)
; ======================================================================
; 섹터 쓰기: SECBUF 를 lf_s 섹터에. Z = 성공
wr_sec:         push    ix
                ld      de,SECBUF
                ld      c,F_SETDTA
                call    BDOS
                ld      hl,(lf_sh)
                ld      a,h
                or      l
                jr      nz,ws_drv               ; 윗 16 비트가 0 이 아니면 Nextor _WRDRV
                ld      de,(lf_s)
                ld      h,1
                ld      a,(lf_drv)
                ld      l,a
                ld      c,F_WRABS
                call    BDOS
                pop     ix
                or      a
                ret
ws_drv:         ld      a,(lf_ord)
                ld      hl,(lf_sh)
                ld      de,(lf_s)
                or      a
                jr      z,ws_o
                ex      de,hl
ws_o:           ld      b,1
                ld      a,(lf_drv)
                ld      c,F_WRDRV
                call    BDOS
                pop     ix
                or      a
                ret

; HL = 항목 (조각 모으기 모드일 때는 LFRAW 에 32 바이트씩 붙임)
lf_raw_add:     ld      a,(lf_mode)
                cp      1
                ret     nz
                ld      a,(lf_rawn)
                cp      20
                ret     nc
                push    hl
                ld      l,a
                ld      h,0
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                ld      de,LFRAW
                add     hl,de
                ex      de,hl
                pop     hl
                ld      bc,32
                ldir
                ld      a,(lf_rawn)
                inc     a
                ld      (lf_rawn),a
                ret

; HL = 폴더 경로, DE = 8.3 이름(11) → 찾으면 lf_found = 1 로 하고 LFRAW 에 조각을 모음
lf_capture:     ld      (lf_tgt),de
                xor     a
                ld      (lf_found),a
                ld      (lf_rawn),a
                ld      a,1
                ld      (lf_mode),a
                ld      de,lf_proc
                ld      (lf_hook),de
                call    lf_geom
                ret     nz
                jp      lf_walk

; HL = 폴더 경로, DE = 8.3 이름(11) → in_placed = 1 이면 LFRAW 의 조각을 씀
lf_install:     ld      a,(lf_rawn)
                or      a
                ret     z
                ld      (lf_tgt),de
                inc     a
                ld      (in_need),a             ; 필요한 칸 수 = 조각 수 + 8.3 항목 1
                xor     a
                ld      (in_found),a
                ld      (in_placed),a
                ld      a,2
                ld      (lf_mode),a
                ld      de,h_find
                ld      (lf_hook),de
                call    lf_geom
                ret     nz
                call    lf_walk                 ; 1) 8.3 항목 찾기
                ld      a,(in_found)
                or      a
                ret     z
                ld      hl,(lf_tgt)             ; 조각들의 검사합
                call    lf_sum
                ld      (in_sum),a
                xor     a
                ld      (lf_done),a
                ld      (rn_len),a
                ld      de,h_run
                ld      (lf_hook),de
                call    lf_walk                 ; 2) 비어 있는 칸에 쓰기
                ld      a,(in_placed)
                or      a
                ret     z
                ld      hl,(in_sa)              ; 3) 옛 8.3 자리가 다른 섹터에 있었으면 지움 (E5)
                ld      de,(in_ps)
                or      a
                sbc     hl,de
                jr      z,li_fl
                ld      hl,(in_sah)
                ld      (lf_sh),hl
                ld      hl,(in_sa)
                ld      (lf_s),hl
                call    rd_sec
                jr      nz,li_fl
                ld      a,(in_ia)
                call    slot_ptr
                ld      (hl),0E5h
                call    wr_sec
li_fl:          ld      b,0FFh                  ; 디스크에 쓰기 및 DOS 쪽도 마무리
                ld      c,F_FLUSH
                call    BDOS
                ret

; A = 줄 번호(0~15) → HL = SECBUF 안의 줄 주소
slot_ptr:       ld      l,a
                ld      h,0
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                ld      de,SECBUF
                add     hl,de
                ret

; 섹터 읽기 hook 1: 8.3 항목(lf_tgt)을 찾아 섹터·칸·내용을 기록
h_find:         ld      hl,SECBUF
                ld      b,16
                ld      c,0
hf_l:           ld      a,(hl)
                or      a
                jr      z,hf_stop
                cp      0E5h
                jr      z,hf_nx
                push    hl
                ld      de,11
                add     hl,de
                ld      a,(hl)
                pop     hl
                cp      0Fh
                jr      z,hf_nx
                push    hl
                push    bc
                ld      de,(lf_tgt)
                ld      b,11
hf_c:           ld      a,(de)
                cp      (hl)
                jr      nz,hf_no
                inc     de
                inc     hl
                djnz    hf_c
                pop     bc
                pop     hl
                ld      a,c
                ld      (in_ia),a
                ld      de,(lf_s)
                ld      (in_sa),de
                ld      de,(lf_sh)
                ld      (in_sah),de
                ld      de,lf_orig
                ld      bc,32
                ldir
                ld      a,1
                ld      (in_found),a
                ld      (lf_done),a
                ret
hf_no:          pop     bc
                pop     hl
hf_nx:          ld      de,32
                add     hl,de
                inc     c
                djnz    hf_l
                ret
hf_stop:        ld      a,1
                ld      (lf_done),a
                ret

; HL = 줄 주소, C = 줄 번호 → A = 1 이면 비어 있음 (지워졌거나 끝)
slot_free:      ld      a,(hl)
                or      a
                jr      z,sf_y
                cp      0E5h
                jr      z,sf_y
                ld      hl,(lf_s)
                ld      de,(in_sa)
                or      a
                sbc     hl,de
                jr      nz,sf_n
                ld      a,(in_ia)
                cp      c
                jr      nz,sf_n
sf_y:           ld      a,1
                ret
sf_n:           xor     a
                ret

; 섹터 읽기 hook 2: 대상 폴더 안에서 in_need 칸이 비어 있는 곳을 찾아 조각들 + 8.3 항목을 씀
h_run:          xor     a
                ld      (rn_len),a
                ld      hl,SECBUF
                ld      b,16
                ld      c,0
hr_l:           push    hl
                push    bc
                call    slot_free
                pop     bc
                pop     hl
                or      a
                jr      z,hr_used
                ld      a,(rn_len)
                or      a
                jr      nz,hr_inc
                ld      a,c
                ld      (rn_st),a
                xor     a
hr_inc:         inc     a
                ld      (rn_len),a
                ld      d,a
                ld      a,(in_need)
                cp      d
                jr      z,hr_place
                jr      hr_nx
hr_used:        xor     a
                ld      (rn_len),a
hr_nx:          ld      de,32
                add     hl,de
                inc     c
                djnz    hr_l
                ret
hr_place:       ld      a,(rn_st)               ; 조각들을 rn_st 칸부터 쓰고 이어서 8.3 항목
                call    slot_ptr
                ex      de,hl
                ld      a,(lf_rawn)
                ld      b,a
                ld      hl,LFRAW
hr_pl:          push    bc
                ld      bc,32
                ldir
                push    hl
                ld      hl,-19                  ; 방금 쓴 조각의 검사합 자리(+13)
                add     hl,de
                ld      a,(in_sum)
                ld      (hl),a
                pop     hl
                pop     bc
                djnz    hr_pl
                ld      hl,lf_orig
                ld      bc,32
                ldir
                ; 옛 8.3 자리가 이 섹터의 범위 밖이면 지움
                ld      hl,(lf_s)
                ld      (in_ps),hl
                ld      de,(in_sa)
                or      a
                sbc     hl,de
                jr      nz,hr_wr
                ld      a,(in_ia)
                ld      b,a
                ld      a,(rn_st)
                cp      b
                jr      z,hr_wr                 ; 범위 시작과 같음: 씀
                jr      nc,hr_out               ; rn_st > in_ia : 범위 밖
                ld      c,a
                ld      a,(in_need)
                add     a,c                     ; 범위 끝을 넘음 검사
                cp      b
                jr      z,hr_out
                jr      c,hr_out                ; in_ia >= 끝: 범위 밖
                jr      hr_wr                   ; 범위 안: 씀
hr_out:         ld      a,b
                call    slot_ptr
                ld      (hl),0E5h
hr_wr:          call    wr_sec
                ld      a,1
                ld      (in_placed),a
                ld      (lf_done),a
                ret

; HL = 폴더 경로, DE = 8.3 이름(11, 첫 글자는 아래 글자). 지워진 긴 이름 조각을 지움
lf_erase:       ld      (lf_tgt),de
                xor     a
                ld      (lf_live),a             ; 0 = 지워진 항목을 못 찾음
                jr      lf_er_go
; HL = 폴더 경로, DE = 찾아 둘 8.3 이름(11), BC = 새 8.3 이름(11, 조각 검사합용)
lf_erase_live:  ld      (lf_tgt),de
                ld      (lf_chkn),bc
                ld      a,1
                ld      (lf_live),a
lf_er_go:       xor     a
                ld      (lf_found),a
                ld      (pn_n),a
                ld      a,3
                ld      (lf_mode),a
                ld      de,h_erase
                ld      (lf_hook),de
                call    lf_geom
                ret     nz
                call    lf_walk
                ld      a,(lf_found)
                or      a
                ret     z
                ld      a,(pn_n)
                ld      b,a
                ld      hl,pn_list
le_m:           push    bc
                push    hl
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                inc     hl
                ld      a,(hl)
                call    er_one
                pop     hl
                ld      de,3
                add     hl,de
                pop     bc
                djnz    le_m
                ld      b,0FFh
                ld      c,F_FLUSH
                jp      BDOS
er_one:         ld      (er_slot),a
                ld      (lf_s),de
                ex      de,hl
                call    rd_sec
                ret     nz
                ld      a,(er_slot)
                call    slot_ptr
                ld      (hl),0E5h
                jp      wr_sec

; 섹터 읽기 hook 3: 모든 조각 안에 있는 "지워진 8.3 항목"(이름 2~11 글자가 같은 것)을 찾음
h_erase:        ld      hl,SECBUF
                ld      b,16
                ld      c,0
he_l:           push    hl
                push    bc
                call    he_one
                pop     bc
                pop     hl
                ld      a,(lf_done)
                or      a
                ret     nz
                ld      de,32
                add     hl,de
                inc     c
                djnz    he_l
                ret
he_one:         ld      a,(hl)
                or      a
                jr      nz,he_n0
                ld      a,1
                ld      (lf_done),a
                ret
he_n0:          cp      0E5h
                jp      z,he_del
                push    hl
                ld      de,11
                add     hl,de
                ld      a,(hl)
                pop     hl
                cp      0Fh
                jr      z,he_pc
                ld      a,(lf_live)             ; 찾아 두는 항목 찾기 모드: 이름이 같고 조각의 검사합이 맞으면 찾음
                or      a
                jr      z,he_norm
                ld      a,(pn_n)
                or      a
                jr      z,he_norm
                ld      a,(pn_seq)
                cp      1
                jr      nz,he_norm
                ld      de,(lf_tgt)
                ld      b,11
he_lc:          ld      a,(de)
                cp      (hl)
                jr      nz,he_norm
                inc     de
                inc     hl
                djnz    he_lc
                ld      hl,(lf_chkn)
                call    lf_sum
                ld      b,a
                ld      a,(pn_chk)
                cp      b
                jr      nz,he_norm
                ld      a,1
                ld      (lf_found),a
                ld      (lf_done),a
                ret
he_norm:        xor     a                       ; 보통 항목: 모은 조각을 버림
                ld      (pn_n),a
                ret
he_pc:          ld      a,(hl)
                and     40h
                jr      z,hx_mid
                xor     a                       ; 새 묶음을 시작
                ld      (pn_n),a
                ld      a,(hl)
                and     3Fh
                ld      (pn_seq),a
                push    hl
                ld      de,13
                add     hl,de
                ld      a,(hl)
                ld      (pn_chk),a
                pop     hl
                jr      hx_add
hx_mid:         ld      a,(pn_n)
                or      a
                ret     z
                ld      a,(pn_seq)
                dec     a
                ld      d,a
                ld      a,(hl)
                and     3Fh
                cp      d
                jr      nz,hx_reset
                ld      (pn_seq),a
                push    hl
                ld      de,13
                add     hl,de
                ld      a,(pn_chk)
                cp      (hl)
                pop     hl
                jr      nz,hx_reset
hx_add:         ld      a,(pn_n)
                cp      20
                ret     nc
                ld      l,a
                ld      h,0
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,de                   ; × 3
                ld      de,pn_list
                add     hl,de
                ld      de,(lf_s)
                ld      (hl),e
                inc     hl
                ld      (hl),d
                inc     hl
                ld      (hl),c
                ld      a,(pn_n)
                inc     a
                ld      (pn_n),a
                ret
hx_reset:       xor     a
                ld      (pn_n),a
                ret
he_del:         ld      a,(lf_live)
                or      a
                jr      nz,he_rs                ; 찾아 두는 항목 모드에서는 지워진 항목도 처리
                ld      a,(pn_n)
                or      a
                ret     z
                ld      a,(pn_seq)
                cp      1
                jr      nz,he_rs
                inc     hl
                ld      de,(lf_tgt)
                inc     de
                ld      b,10
hd_c:           ld      a,(de)
                cp      (hl)
                jr      nz,he_rs
                inc     de
                inc     hl
                djnz    hd_c
                ld      hl,(lf_tgt)
                call    lf_sum
                ld      b,a
                ld      a,(pn_chk)
                cp      b
                jr      nz,he_rs
                ld      a,1
                ld      (lf_found),a
                ld      (lf_done),a
                ret
he_rs:          xor     a
                ld      (pn_n),a
                ret

; HL = 항목 → DE 에 "보이는 이름"(긴 이름이 있으면 그것, 없으면 NAME.EXT) ASCIIZ
get_ds:         push    de
                push    hl
                ld      de,EN_LFN
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                pop     hl
                ld      a,d
                or      e
                jr      z,gd_83
                ex      de,hl
                pop     de
gd_c:           ld      a,(hl)
                ld      (de),a
                or      a
                ret     z
                inc     hl
                inc     de
                jr      gd_c
gd_83:          call    disp_name
                ld      hl,nmbuf
                pop     de
                jr      gd_c

; HL = 항목 A, DE = 항목 B → Z 이면 보이는 이름이 같음 (대소문자 무시)
names_eq:       push    de
                ld      de,ne_a
                call    get_ds
                pop     hl
                ld      de,ne_b
                call    get_ds
                ld      hl,ne_a
                ld      de,ne_b
ne_l:           ld      a,(hl)
                call    up_a
                ld      b,a
                ld      a,(de)
                call    up_a
                cp      b
                ret     nz
                or      a
                ret     z
                inc     hl
                inc     de
                jr      ne_l

; A = 글자 → 대문자로
up_a:           cp      'a'
                ret     c
                cp      'z'+1
                ret     nc
                and     0DFh
                ret

; IX = 패널, HL = 이름 글(ASCIIZ) → HL = 긴 이름이 같은 항목 (없으면 0)
find_lfn:       ld      (fl_s),hl
                ld      l,(ix+PS_ENT)
                ld      h,(ix+PS_ENT+1)
                ld      c,(ix+PS_COUNT)
                ld      b,(ix+PS_COUNT+1)
fl_l:           ld      a,b
                or      c
                jr      z,fl_none
                push    bc
                push    hl
                ld      de,EN_LFN
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ld      a,d
                or      e
                jr      z,fl_no
                ex      de,hl
                ld      de,(fl_s)
fl_c:           ld      a,(hl)
                call    up_a
                ld      b,a
                ld      a,(de)
                call    up_a
                cp      b
                jr      nz,fl_no
                or      a
                jr      z,fl_yes
                inc     hl
                inc     de
                jr      fl_c
fl_yes:         pop     hl
                pop     bc
                ret
fl_no:          pop     hl
                pop     bc
                ld      de,ENSZ
                add     hl,de
                dec     bc
                jr      fl_l
fl_none:        ld      hl,0
                ret

; 원본 항목(cp_ent)의 긴 이름(또는 8.3)에서 겹치지 않는 8.3 별칭(BASE~N.EXT)을 만든다.
; 대상 패널(cp_dst)에 같은 별칭이 없을 때까지 N = 1~9. 못 찾으면 NZ
gen_alias:      ld      hl,(cp_ent)
                ld      de,ga_src
                call    get_ds                  ; 원본 보이는 이름
; (ga_src 에 이름을 넣어 두고 부르기도 함: 폴더 만들기·이름 바꾸기)
gen_alias2:     ld      hl,ga_src               ; 마지막 '.' 찾기
                ld      de,0
gd_f:           ld      a,(hl)
                or      a
                jr      z,gd_fe
                cp      '.'
                jr      nz,gd_fn
                ld      d,h
                ld      e,l
gd_fn:          inc     hl
                jr      gd_f
gd_fe:          ld      (ga_dot),de
                ld      hl,ga_ext               ; 확장자 (대문자, 영문·숫자 3 글자)
                ld      (hl),' '
                ld      de,ga_ext+1
                ld      bc,2
                ldir
                ld      hl,ga_base              ; 기본 이름 6 글자까지
                ld      (hl),' '
                ld      de,ga_base+1
                ld      bc,5
                ldir
                xor     a
                ld      (ga_bn),a
                ld      hl,ga_src
gd_b:           ld      a,(hl)
                or      a
                jr      z,gd_be
                push    hl
                ld      de,(ga_dot)
                ld      a,d
                or      e
                jr      z,gd_nodot
                or      a
                sbc     hl,de
                pop     hl
                jr      z,gd_be                 ; 마지막 '.' 없음
                jr      gd_chr
gd_nodot:       pop     hl
gd_chr:         ld      a,(hl)
                call    ga_chr
                or      a
                jr      z,gd_bs
                ld      c,a
                ld      a,(ga_bn)
                cp      6
                jr      nc,gd_bs
                ld      e,a
                ld      d,0
                inc     a
                ld      (ga_bn),a
                push    hl
                ld      hl,ga_base
                add     hl,de
                ld      (hl),c
                pop     hl
gd_bs:          inc     hl
                jr      gd_b
gd_be:          ld      de,(ga_dot)
                ld      a,d
                or      e
                jr      z,gd_noext
                ex      de,hl
                inc     hl
                ld      b,3
                ld      de,ga_ext
gd_e:           ld      a,(hl)
                or      a
                jr      z,gd_noext
                call    ga_chr
                or      a
                jr      z,gd_en
                ld      (de),a
                inc     de
                dec     b
                jr      z,gd_noext
gd_en:          inc     hl
                jr      gd_e
gd_noext:       ld      a,(ga_bn)
                or      a
                jr      nz,gd_try
                ld      hl,txt_filebase         ; 영문·숫자가 하나도 없으면 "FILE"
                ld      de,ga_base
                ld      bc,4
                ldir
                ld      a,4
                ld      (ga_bn),a
gd_try:         ld      a,'1'
                ld      (ga_n),a
gd_t:           ld      hl,cp_alias             ; 별칭 = BASE + ~N + 공백, 확장자
                ld      (hl),' '
                ld      de,cp_alias+1
                ld      bc,7
                ldir
                ld      a,(ga_bn)
                ld      c,a
                ld      b,0
                ld      hl,ga_base
                ld      de,cp_alias
                ldir
                ld      a,'~'
                ld      (de),a
                inc     de
                ld      a,(ga_n)
                ld      (de),a
                ld      hl,ga_ext
                ld      de,cp_alias+8
                ld      bc,3
                ldir
                ld      ix,(cp_dst)
                ld      de,cp_alias
                call    find_n11
                ld      a,h
                or      l
                ret     z                       ; 겹치지 않음
                ld      de,(gn_skip)            ; 이름을 바꾸는 중이면 자신과만 겹치는 것은 괜찮음
                or      a
                sbc     hl,de
                ret     z
                ld      a,(ga_n)
                inc     a
                ld      (ga_n),a
                cp      ':'
                jr      c,gd_t
                or      1                       ; 실패 (NZ)
                ret
; A = 글자 → 영문·숫자·허용 기호이면 대문자로, 아니면 0
ga_chr:         cp      '0'
                jr      c,gc_no
                cp      '9'+1
                ret     c
                cp      'A'
                jr      c,gc_no
                cp      'Z'+1
                ret     c
                cp      'a'
                jr      c,gc_no
                cp      'z'+1
                jr      nc,gc_no
                and     0DFh
                ret
gc_no:          xor     a
                ret

; inbuf(ASCII + 한글 쌍) -> CPBUF 에 UCS-2 (글자마다 낮은·높은 바이트), lb_un = 글자 수 (한글 = AC00h + 번호)
lb_ucs:         ld      hl,inbuf
                ld      de,CPBUF
                ld      b,0
lu_l:           ld      a,(hl)
                or      a
                jr      z,lu_e
                cp      80h
                jr      c,lu_a
                inc     hl
                ld      a,(hl)
                cp      80h
                jr      c,lu_q                  ; 짝 없는 높은 바이트 = '?'
                push    hl
                sub     80h
                ld      c,a                     ; 번호 AND 7Fh
                dec     hl
                ld      a,(hl)
                sub     80h                     ; 번호 >> 7
                ld      h,a
                ld      l,0
                srl     h
                rr      l
                ld      a,l
                or      c
                ld      l,a
                ld      a,h
                add     a,0ACh
                ld      h,a
                ld      a,l
                ld      (de),a
                inc     de
                ld      a,h
                ld      (de),a
                inc     de
                pop     hl
                inc     hl
                inc     b
                jr      lu_l
lu_q:           ld      a,'?'
                jr      lu_s
lu_a:           inc     hl
lu_s:           ld      (de),a
                inc     de
                xor     a
                ld      (de),a
                inc     de
                inc     b
                jr      lu_l
lu_e:           ld      a,b
                ld      (lb_un),a
                ret

; 긴 이름 조각 안의 글자 위치 (5 + 6 + 2 글자)
lfn_off:        db      1,3,5,7,9,14,16,18,20,22,24,28,30

; ======================================================================
; 복사 (INS): 고른 항목(없으면 커서 항목)을 활성 패널의 폴더 → 반대 패널의 폴더로.
; - 같은 이름이 있으면 물음 (Y 덮어씀 / N 건너뜀 / A 모두 덮어씀 / ESC 그만)
; - 날짜·시간·속성은 원본 그대로
; - (긴 이름 붙이기·폴더 복사는 다음 단계)
; ======================================================================
do_copy:        call    cur_ix
                ld      (cp_src),ix
                ld      a,(act)
                xor     1
                call    panel_ix
                ld      (cp_dst),ix
                ld      ix,(cp_src)
                ; 같은 디렉터리인가
                ld      hl,(cp_src)
                ld      de,PS_PATH
                add     hl,de
                ex      de,hl
                ld      hl,(cp_dst)
                ld      bc,PS_PATH
                add     hl,bc
                call    str_eq                  ; HL = DE 글이 같은가
                jr      nz,cp_diff
                ld      hl,txt_same
                ld      (msg_p),hl
                ret
cp_diff:        ; 복사 버퍼 크기 = TPA 위쪽까지 남는 만큼 (512 의 배수)
                ld      hl,(0006h)
                ld      de,-0600h
                add     hl,de
                ld      de,CPBUF
                or      a
                sbc     hl,de
                jr      c,cp_nomem
                ld      a,h
                and     0FEh
                ld      h,a
                ld      l,0
                ld      (cp_bufsz),hl
                ld      a,h
                cp      8
                jr      nc,cp_ok
cp_nomem:       ld      hl,txt_nomem2
                ld      (msg_p),hl
                ret
cp_ok:          xor     a
                ld      (ow_all),a
                ld      (cp_abort),a
                ld      hl,0
                ld      (cp_cnt),hl
                ld      ix,(cp_src)
                ld      a,(ix+PS_SEL)
                or      (ix+PS_SEL+1)
                jr      nz,cp_many
                ld      a,(ix+PS_COUNT)         ; 고른 항목이 없으면 커서 항목 하나
                or      (ix+PS_COUNT+1)
                jr      z,cp_fin
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                call    is_dotdot
                jr      z,cp_fin
                call    cp_one
                jr      cp_fin
cp_many:        call    selmap_build            ; 고른 항목을 번호 맵으로 (폴더를 복사하면 목록이 다시 읽혀 표시가 지워지므로)
                ld      hl,0
                ld      (cp_i),hl
cp_ml:          ld      a,(cp_abort)
                or      a
                jr      nz,cp_fin
                ld      ix,(cp_src)
                ld      hl,(cp_i)
                ld      e,(ix+PS_COUNT)
                ld      d,(ix+PS_COUNT+1)
                or      a
                sbc     hl,de
                jr      nc,cp_fin
                ld      hl,(cp_i)
                call    selmap_test
                jr      z,cp_nx
                ld      ix,(cp_src)
                ld      hl,(cp_i)
                call    ptr_get
                call    cp_one
cp_nx:          ld      hl,(cp_i)
                inc     hl
                ld      (cp_i),hl
                jr      cp_ml
cp_fin:         ld      ix,(cp_dst)             ; 반대 패널을 다시 읽고 원본의 표시는 지움
                call    panel_load
                ld      ix,(cp_src)
                call    sel_clear
                ld      a,(cp_abort)
                or      a
                ret     nz
                ld      hl,(msg_p)              ; 경고가 있으면 그것을 표시
                ld      a,h
                or      l
                ret     nz
                ld      de,errbuf               ; "Copied N file(s)"
                ld      hl,txt_copied
                call    copy_z
                ld      hl,(cp_cnt)
                call    put_dec16
                ld      hl,txt_files
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,errbuf
                ld      (msg_p),hl
                ret

; HL = 원본 항목 하나를 복사 (폴더는 아직 건너뜀)
cp_one:         ld      (cp_ent),hl
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                jr      z,co_file
                ld      hl,(cp_ent)
                jp      copy_folder
co_file:        ld      hl,(cp_ent)
                call    disp_name               ; nmbuf = 원본 NAME.EXT
                ld      hl,(cp_src)             ; 원본 경로
                ld      de,PS_PATH
                add     hl,de
                ld      de,srcpath
                call    mkpath
                ld      hl,(cp_ent)             ; 대상 별칭 = 원본 8.3 이름 (겹치면 아래에서 바꿈)
                ld      de,cp_alias
                ld      bc,11
                ldir
                xor     a
                ld      (cp_over),a
                ; (1) 대상에 같은 8.3 이름이 있는가
                ld      ix,(cp_dst)
                ld      de,cp_alias
                call    find_n11
                ld      a,h
                or      l
                jr      z,co_lfn
                ld      (cp_hit),hl
                ex      de,hl
                ld      hl,(cp_ent)
                call    names_eq                ; 보이는 이름도 같으면 같은 파일
                jr      nz,co_gen
                ld      a,1
                ld      (cp_over),a
                jr      co_alias
co_gen:         call    gen_alias               ; 이름이 다른데 8.3 이 겹침 → 새 별칭 (BASE~N)
                jr      nz,co_noalias
                jr      co_alias
co_lfn:         ld      hl,(cp_ent)             ; (2) 8.3 은 다르지만 긴 이름이 같은 항목이 대상에 있는가
                ld      de,EN_LFN
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ld      a,d
                or      e
                jr      z,co_alias
                ex      de,hl
                ld      ix,(cp_dst)
                call    find_lfn
                ld      a,h
                or      l
                jr      z,co_alias
                ld      (cp_hit),hl             ; 덮어쓸 항목과 별칭을 정하고 계속
                ld      de,cp_alias
                ld      bc,11
                ldir
                ld      a,1
                ld      (cp_over),a
co_alias:       ld      a,(cp_over)
                or      a
                jr      z,co_go
                ld      a,(ow_all)
                or      a
                jr      nz,co_del
                ld      hl,(cp_ent)             ; 덮어쓸지 물음 (보이는 이름으로)
                ld      de,ne_a
                call    get_ds
                call    ask_over
                cp      'Y'
                jr      z,co_del
                cp      'A'
                jr      nz,co_notall
                ld      a,1
                ld      (ow_all),a
                jr      co_del
co_notall:      cp      'N'
                jr      nz,co_esc
                ld      hl,cp_skip              ; 건너뜀 (이동에서는 원본을 지우지 않아야 함)
                inc     (hl)
                ret
co_esc:         ld      a,1                     ; ESC: 그만
                ld      (cp_abort),a
                ret
co_noalias:     ld      hl,txt_noalias
                ld      (msg_p),hl
                ret
co_del:         ld      hl,cp_alias             ; 덮어쓸 파일(과 긴 이름 조각)을 먼저 지움
                call    disp_name
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,dstpath
                call    mkpath
                ld      de,dstpath
                ld      c,F_DELETE
                call    BDOS
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,cp_alias
                call    lf_erase
co_go:          ld      hl,cp_alias             ; 대상 파일 경로 (별칭으로)
                call    disp_name
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,dstpath
                call    mkpath
                call    copy_data
                ret     nz
                call    lfn_copy                ; 긴 이름 붙이기
                ld      a,(cp_over)             ; 덮어썼으면 대상 목록에 붙이지 않음 (이미 있음, 겹침 검사용)
                or      a
                call    z,append_dst
                ld      hl,(cp_cnt)
                inc     hl
                ld      (cp_cnt),hl
                ret

; 방금 복사한 파일의 긴 이름: 원본 폴더에서 조각을 그대로 모아 대상 폴더에 씀
lfn_copy:       ld      hl,(cp_ent)
                ld      de,EN_LFN
                add     hl,de
                ld      a,(hl)
                inc     hl
                or      (hl)
                ret     z                       ; 원본에 긴 이름이 없음
                ld      hl,(cp_src)
                ld      de,PS_PATH
                add     hl,de
                ld      de,(cp_ent)             ; 원본 8.3 이름
                call    lf_capture
                ld      a,(lf_found)
                or      a
                jr      nz,lc_inst
                ld      hl,txt_nocap            ; 원본의 긴 이름 조각을 못 찾음 (8.3 이름으로만 복사됨)
                ld      (msg_p),hl
                ret
lc_inst:        ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,cp_alias
                call    lf_install
                ld      a,(in_placed)
                or      a
                ret     nz
lc_no:          ld      hl,txt_nolfn
                ld      (msg_p),hl
                ret

; 대상 패널 목록에 방금 만든 항목을 붙임 (정렬 없이, 마지막에 다시 읽음)
append_dst:     ld      ix,(cp_dst)
                ld      l,(ix+PS_COUNT)
                ld      h,(ix+PS_COUNT+1)
                ld      de,MAXE
                or      a
                sbc     hl,de
                ret     nc
                ld      l,(ix+PS_COUNT)
                ld      h,(ix+PS_COUNT+1)
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,de
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; × 24
                ld      e,(ix+PS_ENT)
                ld      d,(ix+PS_ENT+1)
                add     hl,de
                ex      de,hl                   ; DE = 새 항목 자리
                ld      hl,(cp_ent)
                ld      bc,ENSZ
                ldir                            ; 원본 항목을 그대로 (긴 이름 번지 포함)
                ld      hl,-ENSZ
                add     hl,de
                ex      de,hl
                ld      hl,cp_alias             ; 이름을 별칭으로
                ld      bc,11
                ldir
                ld      a,0
                ld      hl,EN_FLAG-11
                add     hl,de
                ld      (hl),a                  ; 표시 해제
                ld      l,(ix+PS_COUNT)
                ld      h,(ix+PS_COUNT+1)
                inc     hl
                ld      (ix+PS_COUNT),l
                ld      (ix+PS_COUNT+1),h
                ret

; srcpath → dstpath 파일 내용 복사. 돌아오면 Z = 성공. 실패하면 cp_ab...
copy_data:      ld      de,srcpath
                ld      a,1                     ; 읽기 전용으로 열기
                ld      c,F_OPEN
                call    BDOS
                or      a
                jp      nz,cd_err
                ld      a,b
                ld      (h_src),a
                ld      de,dstpath
                xor     a
                ld      b,0
                ld      c,F_CREATE
                call    BDOS
                or      a
                jp      nz,cd_err_s
                ld      a,b
                ld      (h_dst),a
cd_loop:        ld      a,(h_src)
                ld      b,a
                ld      de,CPBUF
                ld      hl,(cp_bufsz)
                ld      c,F_READ
                call    BDOS
                cp      E_EOF
                jr      z,cd_done
                or      a
                jr      nz,cd_err_both
                ld      a,h
                or      l
                jr      z,cd_done
                ld      (cd_n),hl
                ld      a,(h_dst)
                ld      b,a
                ld      de,CPBUF
                ld      hl,(cd_n)
                ld      c,F_WRITE
                call    BDOS
                or      a
                jr      nz,cd_err_both
                jr      cd_loop
cd_done:        ld      hl,(cp_ent)             ; 원본의 시간·날짜를 대상 파일에
                ld      de,EN_TIME              ; (_HFTIME 쓰기: HL = 날짜, IX = 시간 — MSX-DOS 2.20 에서 실험으로 확인)
                add     hl,de                   ; 쓰기용 HL = 날짜, DE = 시간
                ld      e,(hl)
                inc     hl
                ld      d,(hl)                  ; DE = 시간
                inc     hl
                ld      c,(hl)
                inc     hl
                ld      b,(hl)                  ; BC = 날짜
                push    ix
                push    de
                pop     ix                      ; IX = 시간
                ld      h,b
                ld      l,c                     ; HL = 날짜
                ld      a,(h_dst)
                ld      b,a
                ld      a,1
                ld      c,F_HFTIME
                call    BDOS
                pop     ix
                ld      a,(h_dst)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                ld      a,(h_src)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                ld      hl,(cp_ent)             ; 속성 (읽기 전용·숨김·시스템·보관)
                ld      de,EN_ATTR
                add     hl,de
                ld      a,(hl)
                and     27h
                ld      l,a                     ; _ATTR 쓰기: A=1, L=속성, DE=경로 (MSX-DOS 2.20 에서 실험으로 확인)
                ld      de,dstpath
                ld      a,1
                ld      c,F_ATTR
                call    BDOS
                xor     a                       ; 성공: Z
                ret
cd_err_both:    push    af
                ld      a,(h_dst)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                pop     af
cd_err_s:       push    af
                ld      a,(h_src)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                pop     af
cd_err:         call    explain_err
                ld      a,1
                ld      (cp_abort),a
                or      a                       ; NZ
                ret

; HL = 글(ASCIIZ), DE = 글 → Z 이면 같음
str_eq:         ld      a,(de)
                cp      (hl)
                ret     nz
                or      a
                ret     z
                inc     hl
                inc     de
                jr      str_eq

; 경로 = HL(폴더 경로) + '\' + nmbuf → DE
mkpath:         ld      a,(hl)
                or      a
                jr      z,mk_end
                ld      (de),a
                inc     hl
                inc     de
                jr      mkpath
mk_end:         dec     de
                ld      a,(de)
                inc     de
                cp      '\'
                jr      z,mk_nm
                ld      a,'\'
                ld      (de),a
                inc     de
mk_nm:          ld      hl,nmbuf
mk_c:           ld      a,(hl)
                ld      (de),a
                or      a
                ret     z
                inc     hl
                inc     de
                jr      mk_c

; IX = 패널, DE = 이름(11) → HL = 같은 이름의 항목 (없으면 0)
find_n11:       ld      (fn_nm),de
                ld      l,(ix+PS_ENT)
                ld      h,(ix+PS_ENT+1)
                ld      c,(ix+PS_COUNT)
                ld      b,(ix+PS_COUNT+1)
f1_l:           ld      a,b
                or      c
                jr      z,f1_none
                push    bc
                push    hl
                ld      de,(fn_nm)
                ld      b,11
f1_c:           ld      a,(de)
                cp      (hl)
                jr      nz,f1_nx
                inc     de
                inc     hl
                djnz    f1_c
                pop     hl
                pop     bc
                ret
f1_nx:          pop     hl
                pop     bc
                ld      de,ENSZ
                add     hl,de
                dec     bc
                jr      f1_l
f1_none:        ld      hl,0
                ret

; 덮어쓸지 물음 → A = 'Y' 'N' 'A' 또는 0 (ESC)
ask_over:       ld      de,errbuf
                ld      hl,txt_over
                call    copy_z
                ld      hl,ne_a                 ; 호출 전에 get_ds 로 채워 둔 보이는 이름 (패널 칸에 맞게 약 20 글자)
                ld      b,20
                call    copy_lim                ; (0.12.12) 한글 쌍을 쪼개지 않고 20바이트까지
                ld      hl,txt_over2
                call    copy_z
; errbuf 에 만든 질문(DE = 끝)을 보여 주고 기다림 → A = 'Y' 'N' 'A' 또는 0
ask_end3:       ld      bc,txt_ppyna            ; 팝업: 예 / 아니오 / 모두
                jr      ask_c
ask_end:        ld      bc,txt_ppyn             ; 팝업: 예 / 아니오
ask_c:          xor     a
                ld      (de),a
                ld      hl,errbuf
                ld      d,b
                ld      e,c
                call    popup_ask
                push    af
                call    draw_all                ; 팝업 지우기
                pop     af
ao_u:           cp      'Y'
                ret     z
                cp      'N'
                ret     z
                cp      'A'
                ret     z
                xor     a
                ret

; IX = 패널: 모든 항목의 표시를 지움
sel_clear:      ld      a,0
                ld      (ix+PS_SEL),a
                ld      (ix+PS_SEL+1),a
                ld      l,(ix+PS_ENT)
                ld      h,(ix+PS_ENT+1)
                ld      c,(ix+PS_COUNT)
                ld      b,(ix+PS_COUNT+1)
sc_lp:          ld      a,b
                or      c
                ret     z
                push    hl
                ld      de,EN_FLAG
                add     hl,de
                ld      (hl),0
                pop     hl
                ld      de,ENSZ
                add     hl,de
                dec     bc
                jr      sc_lp

; ======================================================================
; 삭제 (DEL): 고른 항목(없으면 커서 항목)을 지움. 먼저 "Delete ...? Y/N" 을 물음.
; 파일: 읽기 전용이면 따로 물음 → 속성을 풀고 _DELETE, 달린 긴 이름 조각도 지움.
; 폴더: 안의 파일을 먼저 지우고, 하위 폴더도 같은 방식으로 지운 뒤 폴더 자체를 _DELETE.
; 고른 폴더의 이름은 (dq)에 모았다가 파일을 다 지운 뒤 처리 (폴더를 지우면 목록이 다시 읽혀 표시가 사라지므로).
; ======================================================================
do_delete:      call    cur_ix
                ld      (cp_src),ix
                ld      a,(act)
                xor     1
                call    panel_ix
                ld      (cp_dst),ix
                ld      ix,(cp_src)
                ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                ret     z
                xor     a
                ld      (dl_one),a
                ld      a,(ix+PS_SEL)
                or      (ix+PS_SEL+1)
                jr      nz,dd_many
                ld      l,(ix+PS_CUR)           ; 커서 항목 하나
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                call    is_dotdot
                ret     z
                ld      (dl_ent),hl
                ld      a,1
                ld      (dl_one),a
                ld      de,ne_a
                call    get_ds
                ld      de,errbuf               ; "Delete NAME? Y/N"
                ld      hl,txt_del
                call    copy_z
                ld      hl,ne_a
                ld      b,20
                call    copy_lim
                ld      hl,txt_yn
                call    copy_z
                jr      dd_ask
dd_many:        ld      de,errbuf               ; "Delete N item(s)? Y/N"
                ld      hl,txt_del
                call    copy_z
                ld      l,(ix+PS_SEL)
                ld      h,(ix+PS_SEL+1)
                call    put_dec16
                ld      hl,txt_items2
                call    copy_z
                ld      hl,txt_yn
                call    copy_z
dd_ask:         call    ask_end
                cp      'Y'
                jr      z,dd_go
                ld      hl,0                    ; 취소: 알림 글자를 지움
                ld      (msg_p),hl
                ret
dd_go:          xor     a
                ld      (cp_abort),a
                ld      (dl_all),a
                ld      (dq_n),a
                ld      (dl_fail),a
                ld      hl,0
                ld      (cp_cnt),hl
                ld      (msg_p),hl
                ld      a,(dl_one)
                or      a
                jr      z,dd_sel
                ld      hl,(dl_ent)
                call    dd_item
                jr      dd_phb
dd_sel:         call    selmap_build
                ld      hl,0
                ld      (cp_i),hl
dd_l:           ld      a,(cp_abort)
                or      a
                jr      nz,dd_phb
                ld      ix,(cp_src)
                ld      hl,(cp_i)
                ld      e,(ix+PS_COUNT)
                ld      d,(ix+PS_COUNT+1)
                or      a
                sbc     hl,de
                jr      nc,dd_phb
                ld      hl,(cp_i)
                call    selmap_test
                jr      z,dd_n
                ld      ix,(cp_src)
                ld      hl,(cp_i)
                call    ptr_get
                call    dd_item
dd_n:           ld      hl,(cp_i)
                inc     hl
                ld      (cp_i),hl
                jr      dd_l
dd_phb:         xor     a                       ; 2) 모아 둔 폴더들
                ld      (dq_i),a
dd_pb:          ld      a,(cp_abort)
                or      a
                jr      nz,dd_fin
                ld      a,(dq_i)
                ld      hl,dq_n
                cp      (hl)
                jr      nc,dd_fin
                ld      l,a
                ld      h,0
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,de
                add     hl,de
                add     hl,de                   ; × 11
                ld      de,dq
                add     hl,de
                ex      de,hl
                ld      ix,(cp_src)
                call    find_n11
                ld      a,h
                or      l
                jr      z,dd_pn
                call    del_tree
dd_pn:          ld      a,(dq_i)
                inc     a
                ld      (dq_i),a
                jr      dd_pb
dd_fin:         ld      ix,(cp_dst)             ; 반대 패널을 다시 읽음 (반대쪽이 지운 폴더 안이거나 같은 드라이브이면 그대로)
                call    panel_load
                ld      a,(dir_err)
                cp      E_NOFIL
                jr      z,dd_d2
                ld      hl,(cp_dst)
                ld      de,PS_PATH+3
                add     hl,de
                ld      (hl),0
                ld      ix,(cp_dst)
                call    panel_load
dd_d2:          ld      hl,(msg_p)
                ld      a,h
                or      l
                jr      nz,dd_ld
                ld      de,errbuf
                ld      hl,txt_deleted
                call    copy_z
                ld      hl,(cp_cnt)
                call    put_dec16
                ld      hl,txt_items2
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,errbuf
                ld      (msg_p),hl
dd_ld:          ld      ix,(cp_src)
                jp      panel_load

; HL = 항목: 파일이면 지우고, 폴더면 이름을 큐에
dd_item:        push    hl
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                pop     hl
                jp      z,del_file
dd_queue:       ld      a,(dq_n)                ; 폴더 이름을 큐에 (이동에서도 사용)
                cp      32
                jr      c,di_q
                ld      hl,txt_dqfull
                ld      (msg_p),hl
                ld      a,1
                ld      (cp_abort),a
                ret
di_q:           push    hl
                ld      l,a
                ld      h,0
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,de
                add     hl,de
                add     hl,de
                ld      de,dq
                add     hl,de
                ex      de,hl
                pop     hl
                ld      bc,11
                ldir
                ld      hl,dq_n
                inc     (hl)
                ret

; HL = 파일 항목 (cp_src 패널의 폴더에 있음)
del_file:       ld      (cp_ent),hl
                call    disp_name
                ld      hl,(cp_src)
                ld      de,PS_PATH
                add     hl,de
                ld      de,srcpath
                call    mkpath
                ld      hl,(cp_ent)             ; 읽기 전용이면 따로 물음
                ld      de,EN_ATTR
                add     hl,de
                bit     0,(hl)
                jr      z,df_go
                ld      a,(dl_all)
                or      a
                jr      nz,df_clr
                ld      hl,(cp_ent)
                ld      de,ne_a
                call    get_ds
                ld      de,errbuf
                ld      hl,txt_ro
                call    copy_z
                ld      hl,ne_a
                ld      b,16
                call    copy_lim
                ld      hl,txt_ro2
                call    copy_z
                call    ask_end3
                cp      'Y'
                jr      z,df_clr
                cp      'A'
                jr      nz,df_n
                ld      a,1
                ld      (dl_all),a
                jr      df_clr
df_n:           cp      'N'
                ret     z                       ; 건너뜀
                ld      a,1                     ; 예: ESC: 그만
                ld      (cp_abort),a
                ret
df_clr:         ld      de,srcpath              ; 읽기 전용 해제 (_ATTR 쓰기: A=1, L=속성)
                ld      a,1
                ld      l,0
                ld      c,F_ATTR
                call    BDOS
df_go:          ld      de,srcpath
                ld      c,F_DELETE
                call    BDOS
                or      a
                jr      z,df_ok
                call    explain_err
                ld      a,1
                ld      (cp_abort),a
                ret
df_ok:          ld      hl,(cp_ent)             ; 긴 이름 조각을 지움
                ld      de,EN_LFN
                add     hl,de
                ld      a,(hl)
                inc     hl
                or      (hl)
                jr      z,df_cnt
                ld      hl,(cp_src)
                ld      de,PS_PATH
                add     hl,de
                ld      de,(cp_ent)
                call    lf_erase
df_cnt:         ld      hl,(cp_cnt)
                inc     hl
                ld      (cp_cnt),hl
                ret

; HL = 폴더 항목 (cp_src 패널의 폴더에 있음): 안을 비우고 폴더 자체를 지움
del_tree:       ld      (cp_ent),hl
                ld      a,(lv_depth)
                cp      MAXD
                jr      c,dt_ok
                ld      hl,txt_deep
                jp      cf_fail
dt_ok:          ld      hl,(cp_ent)
                call    disp_name
                ld      hl,(cp_src)
                ld      de,PS_PATH
                add     hl,de
                ld      de,srcpath
                call    mkpath
                ld      hl,srcpath
                call    strlen
                cp      52
                jr      c,dt_l1
                ld      hl,txt_plong
                jp      cf_fail
dt_l1:          call    lv_slot                 ; 저장: 상위 폴더 경로(64) + 이 폴더 8.3 이름(11) + 긴 이름 여부(1)
                ex      de,hl
                ld      hl,(cp_src)
                ld      bc,PS_PATH
                add     hl,bc
                ld      bc,64
                ldir
                ld      hl,(cp_ent)
                ld      bc,11
                ldir
                ld      hl,(cp_ent)
                ld      bc,EN_LFN
                add     hl,bc
                ld      a,(hl)
                inc     hl
                or      (hl)
                ld      (de),a
                ld      hl,lv_depth
                inc     (hl)
                ld      hl,(cp_src)             ; 이 폴더 안으로
                ld      bc,PS_PATH
                add     hl,bc
                ex      de,hl
                ld      hl,srcpath
                call    strcpy_nul
                ld      ix,(cp_src)
                call    panel_load
                ld      hl,0                    ; 1) 안의 파일
                ld      (cp_i),hl
dt_p1:          ld      a,(cp_abort)
                or      a
                jr      nz,dt_end
                ld      ix,(cp_src)
                ld      hl,(cp_i)
                ld      e,(ix+PS_COUNT)
                ld      d,(ix+PS_COUNT+1)
                or      a
                sbc     hl,de
                jr      nc,dt_p2
                ld      hl,(cp_i)
                call    ptr_get
                call    is_dotdot
                jr      z,dt_n1
                push    hl
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                pop     hl
                jr      nz,dt_n1
                call    del_file
dt_n1:          ld      hl,(cp_i)
                inc     hl
                ld      (cp_i),hl
                jr      dt_p1
dt_p2:          ld      a,(cp_abort)            ; 2) 하위 폴더들
                or      a
                jr      nz,dt_end
                call    first_dir
                ld      a,h
                or      l
                jr      z,dt_end
                call    del_tree
                jr      dt_p2
dt_end:         ld      hl,lv_depth             ; 위로 돌아옴
                dec     (hl)
                call    lv_slot
                ld      de,(cp_src)
                push    hl
                ld      hl,PS_PATH
                add     hl,de
                ex      de,hl
                pop     hl
                ld      bc,64
                ldir
                ld      (dt_nm),hl              ; HL = 저장한 8.3 이름
                ld      a,(cp_abort)
                or      a
                jr      nz,dt_reload
                ld      hl,(dt_nm)              ; 폴더 지우기
                call    disp_name
                ld      hl,(cp_src)
                ld      de,PS_PATH
                add     hl,de
                ld      de,srcpath
                call    mkpath
                ld      de,srcpath
                ld      c,F_DELETE
                call    BDOS
                or      a
                jr      z,dt_dok
                call    explain_err             ; 지우지 못함 (남은 것이 있음 등): 알리고 중단
                ld      a,1
                ld      (cp_abort),a
                jr      dt_reload
dt_dok:         ld      hl,(dt_nm)
                ld      bc,11
                add     hl,bc
                ld      a,(hl)
                or      a
                jr      z,dt_cnt
                ld      hl,(cp_src)
                ld      de,PS_PATH
                add     hl,de
                ld      de,(dt_nm)
                call    lf_erase
dt_cnt:         ld      hl,(cp_cnt)
                inc     hl
                ld      (cp_cnt),hl
dt_reload:      ld      ix,(cp_src)
                jp      panel_load

; cp_src 패널 목록에서 폴더 항목 (".." 제외) → HL (없으면 0)
first_dir:      ld      ix,(cp_src)
                ld      hl,0
                ld      (fd_i),hl
fd_l:           ld      hl,(fd_i)
                ld      e,(ix+PS_COUNT)
                ld      d,(ix+PS_COUNT+1)
                or      a
                sbc     hl,de
                jr      nc,fd_none
                ld      hl,(fd_i)
                call    ptr_get
                call    is_dotdot
                jr      z,fd_n
                push    hl
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                pop     hl
                ret     nz
fd_n:           ld      hl,(fd_i)
                inc     hl
                ld      (fd_i),hl
                jr      fd_l
fd_none:        ld      hl,0
                ret

; ======================================================================
; 이름 입력 · 폴더 만들기(^F) · 이름 바꾸기(^E)
; 입력한 이름이 영문 대문자 8.3 에 그대로 들어가면 DOS 이름이 되고, 아니면 8.3 별칭(대문자 ~N)으로 만듦.
; 긴 이름 조각(UCS-2: 입력 글자 그대로)을 만들어 디렉터리에 씀 (lfn_build + lf_install).
; 이름 바꾸기: DOS 로 8.3 을 바꾼 뒤, 옛 조각(옛 별칭의 검사합)을 지우고 새 조각을 씀.
; ======================================================================
; HL = 질문 글. inbuf 에 처음 글(ASCIIZ), in_len 에 길이. 돌아오면 Z = RETURN
input_line:     ld      (il_pr),hl
                ld      a,1
                ld      (il_full),a             ; (0.12.14) 처음에는 팝업 전체, 그 뒤 글자 칸만 다시 그림
il_draw:        call    hk_wst                  ; (0.12.11) HL = 입력 창에 보일 글의 시작 (한글 쌍을 쪼개지 않음)
                ld      de,errbuf
                call    copy_z
                ld      a,'_'
                ld      (de),a
                inc     de
                xor     a
                ld      (de),a
                ld      hl,(il_pr)              ; 팝업: 질문 / 입력칸 / 안내
                ld      (pp_t1),hl
                ld      hl,errbuf
                ld      (pp_t2),hl
                call    hk_hint                 ; 안내 글 (7H 에서는 한/영 표시)
                ld      (pp_t3),hl
                ld      a,50
                ld      (pp_minw),a
                ld      a,(il_full)
                or      a
                ld      a,3
                jr      z,il_pp
                xor     a
                ld      (il_full),a
il_pp:          ld      (pp_one),a
                call    popup_core
                xor     a
                ld      (pp_one),a
                call    il_wait                 ; (0.12.14) 한/영 키(KANAST)·한글 알파벳 읽기
                or      a
                jr      nz,il_kk
                inc     a                       ; 한/영 상태가 바뀜: 팝업 전체를 다시 그림 (안내 글)
                ld      (il_full),a
                jp      il_draw
il_kk:          call    hk_key                  ; 한/영(TAB)·한글 조합
                jp      c,il_draw
                cp      0Dh
                jp      z,il_ok
                cp      1Bh
                jp      z,il_esc
                cp      08h
                jr      z,il_bs
                cp      K_RIGHT                 ; →: 미리 채운 글을 고쳐 쓰려는 것 (첫 글자를 바꾸지 않게)
                jr      nz,il_nk
                xor     a
                ld      (il_first),a
                jp      il_draw
il_nk:          cp      K_UP                    ; 명령줄(il_raw)에서 ↑↓ = 지난 명령 불러오기 (X86FC.HIS)
                jp      z,il_hup
                cp      K_DOWN
                jp      z,il_hdn
                cp      20h
                jp      c,il_draw
                cp      7Fh
                jp      nc,il_draw
                ld      c,a
                ld      a,(il_raw)              ; 명령줄은 모든 글자 허용
                or      a
                ld      a,c
                jr      nz,il_nb
                ld      hl,txt_badch
                ld      b,9
il_b:           cp      (hl)
                jp      z,il_draw               ; 이름에 넣을 수 없는 글자
                inc     hl
                djnz    il_b
il_nb:
                ld      a,(il_first)            ; 미리 채운 글이 있으면 첫 글자는 바꿈
                or      a
                jr      z,il_ap
                xor     a
                ld      (il_first),a
                ld      (in_len),a
                ld      (inbuf),a
il_ap:          ld      a,(in_len)
                cp      IN_MAX
                jp      nc,il_draw
                ld      e,a
                ld      d,0
                ld      hl,inbuf
                add     hl,de
                ld      (hl),c
                inc     hl
                ld      (hl),0
                inc     a
                ld      (in_len),a
                jp      il_draw
il_bs:          xor     a
                ld      (il_first),a
                ld      a,(in_len)
                or      a
                jp      z,il_draw
                call    hk_clen                 ; 마지막 글자 바이트 수 (한글 = 2)
                ld      b,a
                ld      a,(in_len)
                sub     b
                ld      (in_len),a
                ld      e,a
                ld      d,0
                ld      hl,inbuf
                add     hl,de
                ld      (hl),0
                jp      il_draw
il_hup:         ld      a,(il_raw)
                or      a
                jp      z,il_draw
                ld      a,(hi_i)
                cp      4
                jp      nc,il_draw
                inc     a
                push    af
                call    hi_get
                jr      z,il_hx                 ; 그 칸이 비어 있음: 그대로
                pop     af
                ld      (hi_i),a
                xor     a
                ld      (il_first),a
                jp      il_draw
il_hx:          pop     af
                jp      il_draw
il_hdn:         ld      a,(il_raw)
                or      a
                jp      z,il_draw
                ld      a,(hi_i)
                or      a
                jp      z,il_draw
                dec     a
                ld      (hi_i),a
                jr      nz,il_hd1
                xor     a                       ; 맨 아래 = 빈 입력
                ld      (inbuf),a
                ld      (in_len),a
                jp      il_draw
il_hd1:         call    hi_get
                jp      il_draw

; CPBUF (416바이트 = 104 x 4칸) 를 0 으로 채우고 INI 의 "HIST = 글" 줄들을 차례로 넣음 (맨 앞 = 최근)
hi_load:        ld      hl,CPBUF
                ld      de,CPBUF+1
                ld      bc,415
                ld      (hl),0
                ldir
                xor     a
                ld      (hi_k),a
                ld      hl,CPBUF
                ld      (hi_p),hl
                call    il_open
                ret     nz
hl_l:           call    il_next
                jr      z,hl_e
                ld      de,k_hist
                call    il_key
                jr      nz,hl_l
                call    il_val
                ld      a,(hi_k)
                cp      4
                jr      nc,hl_e
                ld      de,(hi_p)
                ld      b,103
hl_c:           ld      a,(hl)
                or      a
                jr      z,hl_ce
                ld      (de),a
                inc     hl
                inc     de
                djnz    hl_c
hl_ce:          ld      hl,(hi_p)
                ld      de,104
                add     hl,de
                ld      (hi_p),hl
                ld      hl,hi_k
                inc     (hl)
                jr      hl_l
hl_e:           jp      il_close
hi_k:           db      0
hi_p:           dw      0
; A = 칸 번호 1~4 → inbuf·in_len 에 불러옴. 돌아와서 Z = 빈 칸(아무것도 안 바뀜)
hi_get:         push    af
                call    hi_load
                pop     af
                ld      hl,CPBUF-104
                ld      de,104
hg_a:           add     hl,de
                dec     a
                jr      nz,hg_a
                ld      a,(hl)
                or      a
                ret     z
                ld      de,inbuf
                call    copy_z
                ld      hl,inbuf
                call    strlen
                ld      (in_len),a
                or      1
                ret
; inbuf 를 기록 맨 앞에 넣음 (맨 앞과 같으면 그대로)
hi_save:        call    hi_load
                ld      hl,CPBUF
                ld      de,inbuf
hs_c:           ld      a,(de)
                cp      (hl)
                jr      nz,hs_new
                or      a
                ret     z
                inc     hl
                inc     de
                jr      hs_c
hs_new:         ld      hl,CPBUF+311
                ld      de,CPBUF+415
                ld      bc,312
                lddr
                ld      hl,inbuf
                ld      de,CPBUF
                ld      bc,104
                ldir
                ld      hl,k_hist               ; (0.12.14) INI 의 HIST 줄들을 새로 씀
                ld      (iw_key),hl
                ld      hl,iw_mkey
                ld      de,gen_hi
                jp      ini_wr
gen_hi:         ld      hl,CPBUF
                ld      (hi_p),hl
                ld      b,4
gh_l:           push    bc
                ld      hl,(hi_p)
                ld      a,(hl)
                or      a
                jr      z,gh_n
                ld      de,CMDBUF
                push    hl
                ld      hl,txt_hik
                call    copy_z
                pop     hl
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,CMDBUF
                call    iw_put
gh_n:           ld      hl,(hi_p)
                ld      de,104
                add     hl,de
                ld      (hi_p),hl
                pop     bc
                djnz    gh_l
                ret
hi_i:           db      0

il_ok:          call    hk_off
                ld      hl,0
                ld      (msg_p),hl
                xor     a
                ret
il_esc:         call    hk_off
                ld      hl,0
                ld      (msg_p),hl
                or      1
                ret

; inbuf 앞뒤의 공백·점을 없앰 (Windows 처럼)
trim_in:        ld      a,(in_len)
                or      a
                jr      z,ti_l
                dec     a
                ld      e,a
                ld      d,0
                ld      hl,inbuf
                add     hl,de
                ld      a,(hl)
                cp      ' '
                jr      z,ti_cut
                cp      '.'
                jr      nz,ti_l
ti_cut:         ld      (hl),0
                ld      a,e
                ld      (in_len),a
                jr      trim_in
ti_l:           ld      a,(in_len)
                or      a
                ret     z
                ld      a,(inbuf)
                cp      ' '
                ret     nz
                ld      hl,inbuf+1
                ld      de,inbuf
                ld      bc,IN_MAX+1
                ldir
                ld      a,(in_len)
                dec     a
                ld      (in_len),a
                jr      ti_l

; HL = 이름 글 → Z 이면 영문 대문자 8.3 에 그대로 들어감 → na_alias(11)
fit83:          ld      (f8_s),hl
                ld      hl,na_alias
                ld      de,na_alias+1
                ld      (hl),' '
                ld      bc,10
                ldir
                ld      hl,(f8_s)
                ld      de,na_alias
                ld      b,8                     ; 기본 이름 최대 8
f8_l:           ld      a,(hl)
                or      a
                jr      z,f8_end
                cp      '.'
                jr      z,f8_dot
                call    f8_ok
                jr      nz,f8_no
                ld      a,b
                or      a
                jr      z,f8_no                 ; 8 글자 넘음
                ld      a,(hl)
                ld      (de),a
                inc     hl
                inc     de
                dec     b
                jr      f8_l
f8_dot:         ld      a,8
                cp      b
                jr      z,f8_no                 ; 기본 이름이 비어 있음
                inc     hl
                ld      de,na_alias+8
                ld      b,3
f8_e:           ld      a,(hl)
                or      a
                jr      z,f8_ok2
                cp      '.'
                jr      z,f8_no                 ; 점이 둘 이상
                call    f8_ok
                jr      nz,f8_no
                ld      a,b
                or      a
                jr      z,f8_no                 ; 확장자가 3 글자 넘음
                ld      a,(hl)
                ld      (de),a
                inc     hl
                inc     de
                dec     b
                jr      f8_e
f8_end:         ld      a,8                     ; 끝에 왔음: 기본 이름이 비어 있으면 실패
                cp      b
                jr      z,f8_no
f8_ok2:         xor     a
                ret
f8_no:          or      1
                ret
; A = 글자 → Z 이면 8.3 이름에 쓸 수 있는 대문자·숫자·기호
f8_ok:          cp      '0'
                jr      c,f8_sym
                cp      '9'+1
                jr      c,f8_y
                cp      'A'
                jr      c,f8_sym
                cp      'Z'+1
                jr      c,f8_y
f8_sym:         push    hl
                push    bc
                ld      hl,txt_sym83
                ld      b,16
f8_s1:          cp      (hl)
                jr      z,f8_sy
                inc     hl
                djnz    f8_s1
                pop     bc
                pop     hl
                or      1
                ret
f8_sy:          pop     bc
                pop     hl
f8_y:           cp      a
                ret

; inbuf 이름의 8.3 별칭을 정함: need_lfn(0/1), na_alias(11). Z = 성공
choose_alias:   xor     a
                ld      (need_lfn),a
                ld      hl,inbuf
                call    fit83
                jr      nz,ca_lfn
                ret                             ; 이미 대문자 8.3: 별칭 = 이름
ca_lfn:         ld      a,1
                ld      (need_lfn),a
                ld      hl,inbuf                ; 대문자로 바꾼 이름이 8.3 에 들어가면 그것을 별칭으로 (docs → DOCS + 긴 이름 "docs")
                ld      de,inbuf_u
cu_l:           ld      a,(hl)
                call    up_a
                ld      (de),a
                inc     hl
                inc     de
                or      a
                jr      nz,cu_l
                ld      hl,inbuf_u
                call    fit83
                jr      nz,ca_gen
                ld      ix,(cp_dst)
                ld      de,na_alias
                call    find_n11
                ld      a,h
                or      l
                ret     z
                ld      de,(gn_skip)
                or      a
                sbc     hl,de
                ret     z
ca_gen:         ld      hl,inbuf                ; BASE~N 별칭
                ld      de,ga_src
                call    strcpy_nul
                call    gen_alias2
                jr      nz,ca_fail
                ld      hl,cp_alias
                ld      de,na_alias
                ld      bc,11
                ldir
                xor     a
                ret
ca_fail:        or      1
                ret

; inbuf(in_len 글자)의 긴 이름 조각을 LFRAW 에 만듦 (디렉터리 순서: 마지막 조각이 먼저)
lfn_build:      call    lb_ucs                  ; (0.12.11) inbuf -> UCS-2 (CPBUF), lb_un = 글자 수
                ld      a,(lb_un)
                ld      c,0
lb_n:           or      a
                jr      z,lb_nd
                inc     c
                cp      13
                jr      c,lb_nd
                sub     13
                jr      lb_n
lb_nd:          ld      a,c
                ld      (lf_rawn),a
                ld      (lb_seq),a
                ld      hl,LFRAW
                ld      (lb_dst),hl
lb_e:           ld      hl,(lb_dst)             ; 남는 글자는 0 으로
                ld      d,h
                ld      e,l
                inc     de
                ld      (hl),0
                ld      bc,31
                ldir
                ld      hl,(lb_dst)
                ld      a,(lb_seq)
                ld      b,a
                ld      a,(lf_rawn)
                cp      b
                ld      a,b
                jr      nz,lb_nf
                or      40h                     ; 마지막 조각 표시
lb_nf:          ld      (hl),a
                ld      de,11
                add     hl,de
                ld      (hl),0Fh                ; 속성 = 긴 이름 조각
                ld      a,(lb_seq)              ; 이 조각의 첫 글자 번호 = (조각 번호-1) × 13
                dec     a
                ld      b,a
                xor     a
                or      b
                jr      z,lb_b0
                xor     a
lb_bm:          add     a,13
                djnz    lb_bm
lb_b0:          ld      (lb_base),a
                xor     a
                ld      (lb_k),a
lb_c:           ld      hl,lfn_off
                ld      a,(lb_k)
                ld      e,a
                ld      d,0
                add     hl,de
                ld      e,(hl)                  ; 글자 쓸 위치
                ld      d,0
                ld      hl,(lb_dst)
                add     hl,de
                ld      a,(lb_base)
                ld      b,a
                ld      a,(lb_k)
                add     a,b
                ld      b,a                     ; 글자 번호
                ld      a,(lb_un)
                cp      b
                jr      z,lb_nul
                jr      c,lb_pad
                push    hl
                ld      l,b
                ld      h,0
                add     hl,hl
                ld      de,CPBUF
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                pop     hl
                ld      (hl),e                  ; UCS-2: 낮은 바이트, 높은 바이트
                inc     hl
                ld      (hl),d
                jr      lb_nx
lb_nul:         ld      (hl),0                  ; 이름 끝 표시 0000
                inc     hl
                ld      (hl),0
                jr      lb_nx
lb_pad:         ld      (hl),0FFh               ; 나머지는 FFFF
                inc     hl
                ld      (hl),0FFh
lb_nx:          ld      a,(lb_k)
                inc     a
                ld      (lb_k),a
                cp      13
                jr      c,lb_c
                ld      hl,(lb_dst)
                ld      de,32
                add     hl,de
                ld      (lb_dst),hl
                ld      a,(lb_seq)
                dec     a
                ld      (lb_seq),a
                jp      nz,lb_e
                ret

; IX = 패널, HL = 이름 글 → HL = 보이는 이름이 같은 항목 (없으면 0). gn_skip 항목은 제외
find_disp:      ld      (fdn_s),hl
                ld      l,(ix+PS_ENT)
                ld      h,(ix+PS_ENT+1)
                ld      c,(ix+PS_COUNT)
                ld      b,(ix+PS_COUNT+1)
fdn_l:          ld      a,b
                or      c
                jr      z,fdn_none
                push    bc
                push    hl
                ld      de,(gn_skip)
                or      a
                sbc     hl,de
                pop     hl
                jr      z,fdn_nx                ; 찾는 자신은 건너뜀
                push    hl
                ld      de,ne_b
                call    get_ds
                pop     hl
                push    hl
                ld      hl,ne_b
                ld      de,(fdn_s)
fdn_c:          ld      a,(hl)
                call    up_a
                ld      b,a
                ld      a,(de)
                call    up_a
                cp      b
                jr      nz,fdn_no
                or      a
                jr      z,fdn_yes
                inc     hl
                inc     de
                jr      fdn_c
fdn_yes:        pop     hl
                pop     bc
                ret
fdn_no:         pop     hl
fdn_nx:         pop     bc
                ld      de,ENSZ
                add     hl,de
                dec     bc
                jr      fdn_l
fdn_none:       ld      hl,0
                ret

; 활성 패널(cp_dst)을 다시 읽고 na_alias 항목에 커서
reload_at:      ld      hl,na_alias
                call    disp_name
                ld      hl,nmbuf
                ld      de,upname
                call    strcpy_nul
                ld      ix,(cp_dst)
                call    panel_load
                ld      ix,(cp_dst)
                jp      find_exact

do_mkdir:       call    cur_ix
                ld      (cp_dst),ix
                ld      hl,0
                ld      (gn_skip),hl
                xor     a
                ld      (inbuf),a
                ld      (in_len),a
                ld      (il_first),a
                ld      hl,txt_mkq
                call    input_line
                ret     nz
                call    trim_in
                ld      a,(in_len)
                or      a
                ret     z
                ld      ix,(cp_dst)
                ld      hl,inbuf
                call    find_disp
                ld      a,h
                or      l
                jr      z,mdk_nd
                ld      hl,txt_exists
                ld      (msg_p),hl
                ret
mdk_nd:         call    choose_alias
                jr      z,mdk_al
                ld      hl,txt_noalias
                ld      (msg_p),hl
                ret
mdk_al:         ld      hl,na_alias
                call    disp_name
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,dstpath
                call    mkpath
                ld      hl,dstpath
                call    strlen
                cp      52
                jr      c,mdk_len
                ld      hl,txt_plong
                ld      (msg_p),hl
                ret
mdk_len:        ld      de,dstpath
                xor     a
                ld      b,10h
                ld      c,F_CREATE
                call    BDOS
                or      a
                jr      z,mdk_ok
                jp      explain_err
mdk_ok:         ld      a,(need_lfn)
                or      a
                jr      z,mdk_fin
                call    lfn_build
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,na_alias
                call    lf_install
                ld      a,(in_placed)
                or      a
                jr      nz,mdk_fin
                ld      hl,txt_nolfn
                ld      (msg_p),hl
mdk_fin:        jp      reload_at

do_rename:      call    cur_ix
                ld      (cp_dst),ix
                ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                ret     z
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                call    is_dotdot
                ret     z
                ld      (rn_ent),hl
                ld      (gn_skip),hl
                ld      de,inbuf
                call    get_ds                  ; 지금 이름 (긴 이름이 있으면 그것)
                ld      hl,inbuf
                call    strlen
                ld      (in_len),a
rn_hok:
                ld      a,1
                ld      (il_first),a            ; 첫 글자를 바꿈 (BS·글자를 누르면 고쳐 쓰기)
                ld      hl,txt_rnq
                call    input_line
                jp      nz,rn_done
                call    trim_in
                ld      a,(in_len)
                or      a
                jp      z,rn_done
                ld      hl,(rn_ent)             ; 그대로(대소문자까지 같음)면 아무것도 안 함
                ld      de,ne_a
                call    get_ds
                ld      hl,ne_a
                ld      de,inbuf
                call    str_eq
                jp      z,rn_done
                ld      ix,(cp_dst)             ; 같은 이름이 이미 있는가 (자기 자신 제외)
                ld      hl,inbuf
                call    find_disp
                ld      a,h
                or      l
                jr      z,rn_nd
                ld      hl,txt_exists
                ld      (msg_p),hl
                jp      rn_done
rn_nd:          ld      hl,(rn_ent)             ; 새 8.3 이름이 너무 길다
                ld      de,rn_old
                ld      bc,11
                ldir
                ld      hl,(rn_ent)
                ld      de,EN_LFN
                add     hl,de
                ld      a,(hl)
                inc     hl
                or      (hl)
                ld      (rn_hadlfn),a
                call    choose_alias
                jr      z,rn_al
                ld      hl,txt_noalias
                ld      (msg_p),hl
                jp      rn_done
rn_al:          ld      hl,rn_old               ; 별칭이 같으면 DOS 이름 바꾸기는 건너뜀
                ld      de,na_alias
                ld      b,11
rn_cmp:         ld      a,(de)
                cp      (hl)
                jr      nz,rn_dos
                inc     hl
                inc     de
                djnz    rn_cmp
                jr      rn_pieces
rn_dos:         ld      hl,rn_old
                call    disp_name
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,srcpath
                call    mkpath                  ; 대상 경로
                ld      hl,na_alias
                call    disp_name               ; nmbuf = 옛 8.3 이름
                ld      hl,nmbuf
                ld      de,srcpath
                ld      c,F_RENAME
                call    BDOS
                or      a
                jr      z,rn_pieces
                call    explain_err
                jp      rn_done
rn_pieces:      ld      a,(rn_hadlfn)           ; 옛 긴 이름 조각 지우기
                or      a
                jr      z,rn_new
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,na_alias
                ld      bc,rn_old
                call    lf_erase_live
rn_new:         ld      a,(need_lfn)            ; 새 긴 이름 쓰기
                or      a
                jr      z,rn_fin
                call    lfn_build
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,na_alias
                call    lf_install
                ld      a,(in_placed)
                or      a
                jr      nz,rn_fin
                ld      hl,txt_nolfn
                ld      (msg_p),hl
rn_fin:         call    reload_at
rn_done:        ld      hl,0
                ld      (gn_skip),hl
                ret

do_move:        ld      a,5                     ; (0.12.11) 이동은 오버레이 5 (ov_move.inc)
                ld      b,0
                jp      ov_go

; 시작할 때: 자기 경로 알아 두고, 저장된 상태가 있으면 되살림
restore_state:  ld      hl,envn_prog            ; 자기 경로 (셸이 PROGRAM 에 넣어 줌)
                ld      de,SELFPATH
                ld      b,63
                ld      c,F_GENV
                call    BDOS
                ld      a,(SELFPATH)
                or      a
                jr      nz,rs_self
                ld      hl,txt_selfdef
                ld      de,SELFPATH
                call    copy_z
                xor     a
                ld      (de),a
rs_self:        ld      a,(0FF43h)              ; BASIC 에 남은 H.GONE 훅(복귀용)이 있으면 원래대로
                cp      0C3h
                jr      nz,rs_nh
                ld      a,(0FF45h)
                cp      HB>>8
                jr      nz,rs_nh
                ld      hl,0FF43h
                ld      b,5
rs_nhl:         ld      (hl),0C9h
                inc     hl
                djnz    rs_nhl
rs_nh:
rs_nosh:        ld      hl,envn_x86
                ld      de,ENVBUF
                ld      b,255
                ld      c,F_GENV
                call    BDOS
                ld      a,(ENVBUF)
                or      a
                ret     z
                ld      hl,envn_x86             ; 읽었으면 지움 (한 번만 쓰는 값)
                ld      de,rs_empty
                ld      c,F_SENV
                call    BDOS
                call    bat_cur                 ; 지금 도는(나를 띄운) 배치가 아닌 쪽 배치 파일(지난번 것)을 지움
                xor     3
                ld      (txt_runbat+6),a
                ld      hl,txt_runbat
                call    run_path
                ld      de,PROGBUF
                ld      c,F_DELETE
                call    BDOS
                ld      hl,envn_xt              ; 명령줄용 임시 배치 파일이 있으면 지움
                ld      de,CMDBUF
                ld      b,100
                ld      c,F_GENV
                call    BDOS
                ld      a,(CMDBUF)
                or      a
                jr      z,rs_nt
                ld      de,CMDBUF
                ld      c,F_DELETE
                call    BDOS
                ld      hl,CMDBUF               ; 같은 폴더의 BASIC 시작 프로그램(X86FCB.BAS)도 지움
                ld      d,h
                ld      e,l
rs_ls:          ld      a,(hl)
                or      a
                jr      z,rs_le
                cp      '\'
                jr      nz,rs_lc
                ld      d,h
                ld      e,l
rs_lc:          inc     hl
                jr      rs_ls
rs_le:          inc     de
                ex      de,hl
                ld      de,txt_basname
rs_lcp:         ld      a,(de)
                ld      (hl),a
                inc     de
                inc     hl
                or      a
                jr      nz,rs_lcp
                ld      de,CMDBUF
                ld      c,F_DELETE
                call    BDOS
                ld      hl,envn_xt
                ld      de,rs_empty
                ld      c,F_SENV
                call    BDOS
rs_nt:
                ld      hl,ENVBUF+2
                ld      (rs_p0),hl
                call    rs_next
                ld      (rs_n0),hl
                call    rs_next
                ld      (rs_p1),hl
                call    rs_next
                ld      (rs_n1),hl
                call    rs_next
                ld      a,(ENVBUF)
                sub     '0'
                and     1
                ld      (act),a
                ld      ix,pstate
                ld      hl,(rs_p0)
                ld      de,(rs_n0)
                call    rs_panel
                ld      ix,pstate+PSZ
                ld      hl,(rs_p1)
                ld      de,(rs_n1)
rs_panel:       ld      a,(hl)                  ; HL = 경로, DE = 커서 항목 이름 (IX = 패널)
                or      a
                ret     z
                push    de
                push    ix
                pop     de
                push    de
                ld      bc,PS_PATH
                ex      de,hl
                add     hl,bc
                ex      de,hl                   ; DE = 패널 경로 칸
                ld      b,63
rp_c:           ld      a,(hl)
                ld      (de),a
                or      a
                jr      z,rp_ce
                inc     hl
                inc     de
                djnz    rp_c
                xor     a
                ld      (de),a
rp_ce:          pop     ix
                call    panel_load
                pop     hl
                ld      de,upname
                call    strcpy_nul
                jp      find_exact
rs_next:        ld      a,(hl)                  ; '|' 까지 건너뛰며 0 으로 바꿈
                or      a
                ret     z
                cp      '|'
                jr      z,rn_e
                inc     hl
                jr      rs_next
rn_e:           ld      (hl),0
                inc     hl
                ret

; 엔터로 파일을 눌렀을 때 (IX = 패널, en_p = 항목)
run_cur:        xor     a
                ld      (cl_tmp),a
                ld      (hook_sh),a
                ld      hl,(en_p)
                call    disp_name
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      de,RUNPATH              ; RUNPATH = 고른 파일의 전체 경로
                call    mkpath
                push    ix
                call    vl_prep                 ; 같은 확장자의 등록된 프로그램을 모음
                pop     ix
                ld      a,(vl_n)
                cp      2
                jp      nc,vl_manage            ; 둘 이상 → 풀다운으로 고름
                cp      1
                jr      nz,rc_fl
                ld      hl,VLBUF                ; 하나면 그대로 실행
                jr      rc_have
rc_fl:          push    ix
                call    find_link               ; HL = 명령 틀 (없으면 0)
                pop     ix
                ld      a,h
                or      l
                jr      nz,rc_have
                ld      a,(sv_ext)              ; 연결된 프로그램이 없으면 팝업으로 물어봄 → 예 = Ctrl+F3 식 프로그램 목록 팝업
                cp      ' '
                jr      z,rc_noext
                ld      de,errbuf
                ld      hl,txt_nolk1
                call    copy_z
                ld      hl,sv_ext
                ld      b,3
rc_nx:          ld      a,(hl)
                cp      ' '
                jr      z,rc_nxe
                ld      (de),a
                inc     hl
                inc     de
                djnz    rc_nx
rc_nxe:         ld      hl,txt_nolk2
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,errbuf
                call    popup_yn
                jr      nz,rc_nolk
                call    cur_ix
                jp      vl_manage
rc_nolk:        call    draw_all
                ret
rc_noext:       ld      hl,txt_norun            ; (확장자가 없는 파일만 안내 글)
                ld      (msg_p),hl
                ret
txt_nolk1:      db      "No program is linked to .",0
txt_nolk2:      db      "  -  link one now?",0
rc_have:        ld      a,(hl)                  ; (0.12.27) '#' 로 시작하면 내부 명령 (#VIEW, #EDIT)
                cp      '#'
                jp      z,rc_int
                call    expand_cmd              ; CMDBUF = 틀에서 %s → 파일 경로
                call    split_cmd               ; PROGBUF = 프로그램 경로, TAILBUF = 명령줄
rc_go:          ; --- 1) 이번에 쓸 배치 = 지금 도는 배치(X86FC_B)의 다른 쪽 (돌고 있는 배치 파일은 건드리지 않음)
                call    bat_cur
                xor     3
                ld      (txt_runbat+6),a
                ld      (ENVBUF),a
                xor     a
                ld      (ENVBUF+1),a
                ld      hl,envn_xb
                ld      de,ENVBUF
                ld      c,F_SENV
                call    BDOS
                ld      a,(cl_tmp)              ; F5(BASIC): 시작 프로그램 경로를 저장 (돌아와서 지움)
                cp      2
                jr      nz,rg_nt
                ld      hl,envn_xt
                ld      de,PROGBUF
                ld      c,F_SENV
                call    BDOS
rg_nt:          ; --- 2) 배치 내용을 CPBUF 에: 명령(CL_TMP 0 = 프로그램 경로 + 명령줄, 그 밖 = CMDBUF 글) / PAUSE / X86FC 전체 경로
                ld      de,CPBUF
                ld      a,(cl_tmp)
                or      a
                jr      nz,rg_raw
                ld      hl,PROGBUF
                call    cpz_dbl
                ld      hl,TAILBUF              ; 길이, 공백 + 인자..., CR
                ld      b,(hl)
                inc     hl
                call    cpn_dbl
                jr      rg_ce
rg_raw:         ld      hl,CMDBUF
                call    cpz_dbl
rg_ce:          ld      a,13
                ld      (de),a
                inc     de
                ld      a,10
                ld      (de),a
                inc     de
                ld      a,(cl_tmp)              ; 끝난 뒤 "Press any key" 를 할 것(명령줄·사용자 메뉴·F5)은 PAUSE 한 줄
                or      a
                jr      z,rg_np
                ld      hl,txt_pause
                call    copy_z
rg_np:          ld      hl,SELFPATH             ; 마지막 줄 = X86FC 전체 경로 (현재 드라이브·폴더와 무관하게 실행)
                call    copy_z
                ld      a,13
                ld      (de),a
                inc     de
                ld      a,10
                ld      (de),a
                inc     de
                ld      hl,-CPBUF
                add     hl,de
                ld      (rg_len),hl
                ; --- 3) 배치 파일 쓰기
                ld      hl,txt_runbat
                call    run_path                ; PROGBUF = 배치 파일 경로
                ld      de,PROGBUF
                xor     a
                ld      b,0
                ld      c,F_CREATE
                call    BDOS
                or      a
                jp      nz,rg_bad
                ld      a,b
                ld      (h_dst),a
                ld      hl,(rg_len)
                ld      de,CPBUF
                ld      c,F_WRITE
                call    BDOS
                ld      a,(h_dst)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                ; --- 4) 상태 저장: "활성|경로0|이름0|경로1|이름1|"
                ld      de,ENVBUF
                ld      a,(act)
                add     a,'0'
                ld      (de),a
                inc     de
                ld      a,'|'
                ld      (de),a
                inc     de
                ld      ix,pstate
                call    ss_panel
                ld      ix,pstate+PSZ
                call    ss_panel
                xor     a
                ld      (de),a
                ld      hl,envn_x86
                ld      de,ENVBUF
                ld      c,F_SENV
                call    BDOS
                ; --- 5) 현재 드라이브·폴더를 활성 패널의 것으로
                call    cur_ix
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      a,(hl)
                and     0DFh
                sub     'A'
                ld      e,a
                ld      c,F_SELD
                call    BDOS
                call    cur_ix
                push    ix
                pop     de
                ld      hl,PS_PATH
                add     hl,de
                ex      de,hl
                ld      c,F_CHDIR
                call    BDOS
                call    scr_exit
                ; --- 6) 키 버퍼에 ESC + 배치 이름 + RETURN 을 넣고 (ESC = 입력줄 비움) 완전히 끝냄 → COMMAND2 가 배치를 실행
                ld      de,0
                ld      c,F_DEFER
                call    BDOS
                di
                ld      de,0FBF0h
                ld      (GETPNT),de
                ld      a,1Bh
                ld      (de),a
                inc     de
                ld      hl,PROGBUF
rg_k:           ld      a,(hl)
                or      a
                jr      z,rg_ke
                ld      (de),a
                inc     hl
                inc     de
                jr      rg_k
rg_ke:          ld      a,13
                ld      (de),a
                inc     de
                ld      (PUTPNT),de
                ei
                ld      b,0
                ld      c,62h                   ; _TERM
                jp      BDOS
rg_bad:         ld      hl,txt_notmp
                ld      (msg_p),hl
                ret
; A = 지금 도는(나를 띄운) 배치의 번호 글자 ('1' 또는 '2', 환경 변수 X86FC_B, 없으면 '2')
bat_cur:        ld      hl,envn_xb
                ld      de,CMDBUF+240
                ld      b,4
                ld      c,F_GENV
                call    BDOS
                ld      a,(CMDBUF+240)
                cp      '1'
                ret     z
                ld      a,'2'
                ret
; HL = 글(NUL 끝) → DE 에 덧붙임. 배치 안에서 % 는 %% 로 (DE 는 끝으로)
cpz_dbl:        ld      a,(hl)
                or      a
                ret     z
                inc     hl
                cp      '%'
                jr      nz,cz_c
                ld      (de),a
                inc     de
cz_c:           ld      (de),a
                inc     de
                jr      cpz_dbl
; HL = 글, B = 글자 수 → DE 에 덧붙임 (% 는 %%)
cpn_dbl:        ld      a,b
                or      a
                ret     z
                ld      a,(hl)
                inc     hl
                cp      '%'
                jr      nz,cn_c
                ld      (de),a
                inc     de
cn_c:           ld      (de),a
                inc     de
                djnz    cpn_dbl
                ret
; HL = 파일 이름 글 → PROGBUF = X86FC.COM 이 있는 폴더 + 그 이름.
; 키 버퍼(40 칸)에 ESC + 이름 + RETURN 이 들어가야 하므로 37 글자까지, 넘으면 X86FC 가 있는 드라이브의 루트
run_path:       push    hl
                ld      hl,SELFPATH
                ld      de,PROGBUF
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,PROGBUF
                ld      d,h
                ld      e,l                     ; DE = 마지막 '\' 다음 자리 (없으면 맨 앞)
rp_l:           ld      a,(hl)
                inc     hl
                or      a
                jr      z,rp_e
                cp      '\'
                jr      nz,rp_l
                ld      d,h
                ld      e,l
                jr      rp_l
rp_e:           pop     hl                      ; HL = 이름 글
                push    hl
                ld      b,0
rp_n:           ld      a,(hl)                  ; 이름 길이
                or      a
                jr      z,rp_nl
                inc     hl
                inc     b
                jr      rp_n
rp_nl:          ld      hl,-PROGBUF
                add     hl,de                   ; HL = 폴더 글 길이
                ld      a,l
                add     a,b
                cp      38
                jr      c,rp_ok
                ld      a,(PROGBUF+1)           ; 너무 김 → "X:\" 로
                cp      ':'
                jr      nz,rp_ok
                ld      de,PROGBUF+3
                ld      a,'\'
                ld      (PROGBUF+2),a
rp_ok:          pop     hl
                call    copy_z
                xor     a
                ld      (de),a
                ret


; ---- 아래 줄 명령줄 ('/' 키): 입력한 명령을 임시 배치 파일로 만들어 셸이 실행 (DOS 내부 명령도 됨), 끝나면 "Press any key" 후 복귀 ----
do_cmd:         xor     a
                ld      (cmd_ins),a
                jr      dc_start
do_cmd_ins:     ld      a,1                     ; 커서 항목 이름을 미리 넣고 시작
                ld      (cmd_ins),a
dc_start:       xor     a
                ld      (hook_sh),a
                ld      (inbuf),a
                ld      (in_len),a
                ld      (il_first),a
                ld      (cl_tmp),a
                ld      (hi_i),a
                ld      a,(cmd_ins)
                or      a
                jr      z,dc_in
                call    cur_ix
                ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                jr      z,dc_in
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                call    disp_name
                ld      hl,nmbuf
                ld      de,inbuf
                call    strcpy_nul
                ld      hl,nmbuf
                call    strlen
                ld      (in_len),a
dc_in:          ld      a,1
                ld      (il_raw),a
                ld      hl,txt_cmdq
                call    input_line
                push    af
                xor     a
                ld      (il_raw),a
                pop     af
                ret     nz
                ld      a,(in_len)
                or      a
                ret     z
                call    hi_save
dc_make:        ld      hl,inbuf               ; 명령 글 → CMDBUF 그대로 (COMMAND2 가 배치 안에서 실행: DOS 내부 명령도 됨), 끝나면 PAUSE
                ld      de,CMDBUF
                call    copy_z
                xor     a
                ld      (de),a
                ld      a,(cl_tmp)
                cp      2                       ; 2 = F5(BASIC) 그대로 둠
                jr      z,dc_go
                ld      a,1
                ld      (cl_tmp),a
dc_go:          jp      rc_go

; A = 번호 → HL = 항목 자리 (CPBUF + 번호 × 64)
qd_ent:         ld      l,a
                ld      h,0
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                ld      de,CPBUF
                add     hl,de
                ret
; (0.12.14) INI 의 "QDIR = 경로" 줄들을 CPBUF 항목(64바이트)으로 읽음 / 쓸 때는 QDIR 줄들을 지우고 새로 붙임
qd_load:        xor     a
                ld      (qd_n),a
                call    il_open
                ret     nz
ql_l:           call    il_next
                jr      z,ql_e
                ld      de,k_qdir
                call    il_key
                jr      nz,ql_l
                call    il_val
                ld      a,(qd_n)
                cp      20
                jr      nc,ql_e
                push    hl
                call    qd_ent
                ex      de,hl
                pop     hl
                ld      b,63
ql_c:           ld      a,(hl)
                or      a
                jr      z,ql_ce
                ld      (de),a
                inc     hl
                inc     de
                djnz    ql_c
ql_ce:          xor     a
                ld      (de),a
                ld      hl,qd_n
                inc     (hl)
                jr      ql_l
ql_e:           jp      il_close
qd_save:        ld      hl,k_qdir
                ld      (iw_key),hl
                ld      hl,iw_mkey
                ld      de,gen_qd
                jp      ini_wr
gen_qd:         xor     a
                ld      (qd_i),a
gq_l:           ld      a,(qd_i)
                ld      hl,qd_n
                cp      (hl)
                ret     nc
                ld      de,CMDBUF
                ld      hl,txt_qdk
                call    copy_z
                ld      a,(qd_i)
                push    de
                call    qd_ent
                pop     de
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,CMDBUF
                call    iw_put
                ld      hl,qd_i
                inc     (hl)
                jr      gq_l
; HL = 글, A = 줄 → 80 칸 줄로 화면에
qd_line:        ld      (qd_r),a
                push    hl
                ld      hl,hbuf
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                pop     hl
                ld      de,hbuf+2
                call    copy_z
                ld      a,(qd_r)
                call    row_addr
                ld      de,hbuf
                ld      b,80
                jp      scr_writep
; A = 상자 줄 번호(0=맨 위), B = 종류(0 위 1 가운데 2 아래), HL = 글(0 = 없음) → 상자 한 줄 (70 칸, 5 칸부터)
qd_prow:        ld      (qd_pa),a
                push    hl
                ld      c,CH_VBAR
                ld      d,' '
                ld      e,CH_VBAR
                ld      a,b
                or      a
                jr      nz,qp_1
                ld      c,CH_TL
                ld      d,CH_HBAR
                ld      e,CH_TR
                jr      qp_f
qp_1:           cp      2
                jr      nz,qp_f
                ld      c,CH_BL
                ld      d,CH_HBAR
                ld      e,CH_BR
qp_f:           ld      hl,qd_tmp
                ld      (hl),c
                inc     hl
                ld      b,68
qp_l:           ld      (hl),d
                inc     hl
                djnz    qp_l
                ld      (hl),e
                pop     hl
                ld      a,h
                or      l
                jr      z,qp_w
                ld      de,qd_tmp+2
                ld      b,64
qp_c:           ld      a,(hl)
                or      a
                jr      z,qp_w
                ld      (de),a
                inc     hl
                inc     de
                djnz    qp_c
qp_w:           ld      a,(qd_pa)
                ld      hl,qd_y0
                add     a,(hl)
                call    row_addr
                ld      de,5
                add     hl,de
                ld      de,qd_tmp
                ld      b,70
                ld      a,12                    ; (SCREEN 7) 팝업 색
                ld      (s7_pair),a
                call    scr_writep
                xor     a
                ld      (s7_pair),a
                ret
; HL = 글 → 안내 줄 (항목 바로 아래 줄)
qd_hint:        ld      a,(qd_nl)
                inc     a
                ld      b,1
                jp      qd_prow
; Quick-Dir 팝업: 가운데 테두리 상자 (70 칸, 줄 수 = 항목 수 + 3), 항목 · 안내 줄 · 테두리
quickdir:       call    qd_load
                xor     a
                ld      (qd_sel),a              ; (0.12.10) 처음엔 첫 항목 선택
qd_draw:        ld      a,(qd_n)               ; 선택 번호를 목록 길이 안으로 (DEL 뒤 등)
                ld      b,a
                ld      a,(qd_sel)
                cp      b
                jr      c,qd_sk
                ld      a,b
                or      a
                jr      z,qd_sk
                dec     a
qd_sk:          ld      (qd_sel),a
                call    draw_all                ; 밑그림(패널)을 다시 그린 뒤 그 위에 상자
                ld      a,(qd_n)
                or      a
                jr      nz,qd_h1
                inc     a                       ; 비었으면 "없음" 한 줄
qd_h1:          ld      (qd_nl),a
                add     a,3
                ld      b,a
                ld      a,24
                sub     b
                srl     a
                ld      (qd_y0),a
                ld      hl,txt_qdt
                xor     a
                ld      b,a
                call    qd_prow                 ; 위 테두리 + 제목
                xor     a
                ld      (qd_i),a
qd_dl:          ld      a,(qd_i)
                ld      hl,qd_n
                cp      (hl)
                jr      nc,qd_dn
                call    qd_ln
                ld      a,(qd_i)
                inc     a
                ld      (qd_i),a
                jr      qd_dl
; A = 번호: 그 항목 줄 하나를 그림 (선택이면 '>' 표시) (0.12.10)
qd_ln:          ld      (qd_i),a
                ld      hl,qd_sel
                cp      (hl)
                ld      a,' '
                jr      nz,qd_lm
                ld      a,'>'
qd_lm:          ld      de,errbuf
                ld      (de),a
                inc     de
                ld      a,(qd_i)
                cp      10
                jr      c,qd_l1
                add     a,'A'-10
                jr      qd_l2
qd_l1:          add     a,'0'
qd_l2:          ld      (de),a
                inc     de
                ld      a,' '
                ld      (de),a
                inc     de
                ld      a,(qd_i)
                push    de
                call    qd_ent
                pop     de
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,errbuf
                ld      a,(qd_i)
                inc     a
                ld      b,1
                jp      qd_prow
qd_dn:          ld      a,(qd_n)
                or      a
                jr      nz,qd_dh
                ld      hl,txt_qdnone
                ld      a,1
                ld      b,1
                call    qd_prow
qd_dh:          ld      hl,txt_qdh
                call    qd_hint
                ld      a,(qd_nl)
                add     a,2
                ld      b,2
                ld      hl,0
                call    qd_prow                 ; 아래 테두리
qd_key:         ld      a,(qd_n)
                or      a
                jr      nz,qd_k2
qd_k2:          call    wait_key
                cp      1Bh
                ret     z
                cp      K_INS
                jp      z,qd_add
                cp      K_DEL
                jp      z,qd_rm
                cp      K_DOWN                  ; (0.12.10) 위/아래로 고르고 RETURN 으로 이동
                jr      z,qd_dn1
                cp      K_UP
                jr      z,qd_up1
                cp      0Dh
                jr      z,qd_rt
                call    qd_idx                  ; 글쇠 → 번호 (C = 1 이면 해당 없음)
                jr      c,qd_k2
                ld      b,a
                ld      a,(qd_n)
                ld      c,a
                ld      a,b
                cp      c
                jr      nc,qd_k2
                jp      qd_go
qd_dn1:         ld      a,(qd_sel)
                inc     a
                ld      hl,qd_n
                cp      (hl)
                jr      nc,qd_k2
                jr      qd_mv
qd_up1:         ld      a,(qd_sel)
                or      a
                jr      z,qd_k2
                dec     a
qd_mv:          ld      b,a                     ; 예전 선택 줄·새 선택 줄 두 줄만 다시 그림
                ld      a,(qd_sel)
                ld      c,a
                ld      a,b
                ld      (qd_sel),a
                push    bc
                ld      a,c
                call    qd_ln
                pop     bc
                ld      a,b
                call    qd_ln
                jr      qd_k2
qd_rt:          ld      a,(qd_n)
                or      a
                jr      z,qd_k2
                ld      a,(qd_sel)
                jp      qd_go
qd_sel:         db      0
; A = 글쇠 → A = 번호 0~19, 아니면 C 플래그
qd_idx:         cp      '0'
                jr      c,qi_no
                cp      '9'+1
                jr      nc,qi_a
                sub     '0'
                or      a
                ret
qi_a:           and     0DFh
                cp      'A'
                jr      c,qi_no
                cp      'K'
                jr      nc,qi_no
                sub     'A'-10
                or      a
                ret
qi_no:          scf
                ret
qd_add:         ld      a,(qd_n)
                cp      20
                jr      c,qa_ok
                ld      hl,txt_qdfull
                call    qd_hint
                jp      qd_k2
qa_ok:          call    qd_ent
                ex      de,hl
                call    cur_ix
                push    ix
                pop     hl
                ld      bc,PS_PATH
                add     hl,bc
                ld      b,63
qa_c:           ld      a,(hl)
                ld      (de),a
                or      a
                jr      z,qa_e
                inc     hl
                inc     de
                djnz    qa_c
                xor     a
                ld      (de),a
qa_e:           ld      hl,qd_n
                inc     (hl)
                call    qd_save
                jp      qd_draw
qd_rm:          ld      hl,txt_qdrm
                call    qd_hint
                call    wait_key
                call    qd_idx
                jp      c,qd_draw
                ld      b,a
                ld      a,(qd_n)
                ld      c,a
                ld      a,b
                cp      c
                jp      nc,qd_draw
                ld      (qd_i),a
qr_l:           ld      a,(qd_i)                ; 뒤 항목을 한 칸씩 앞으로
                inc     a
                ld      hl,qd_n
                cp      (hl)
                jr      nc,qr_e
                call    qd_ent
                push    hl
                ld      a,(qd_i)
                call    qd_ent
                ex      de,hl
                pop     hl
                ld      bc,64
                ldir
                ld      a,(qd_i)
                inc     a
                ld      (qd_i),a
                jr      qr_l
qr_e:           ld      hl,qd_n
                dec     (hl)
                call    qd_save
                jp      qd_draw
qd_go:          push    af                      ; A = 번호: 활성 패널을 그 폴더로
                call    cur_ix
                call    path_save
                pop     af
                call    qd_ent
                push    ix
                pop     de
                ld      bc,PS_PATH
                ex      de,hl
                add     hl,bc
                ex      de,hl                   ; DE = 패널 경로 칸, HL = 항목
                ld      bc,64
                ldir
                call    panel_load
                ld      a,(dir_err)
                cp      E_NOFIL
                ret     z
                call    explain_err
                call    path_restore
                jp      panel_load

find_link:      xor     a
                ld      (fl_ok),a
                ld      hl,(en_p)
                ld      de,EN_EXT
                add     hl,de
                ld      (lk_ep),hl
                call    il_open                 ; (0.12.14) INI 전체를 줄 단위로 읽음 (크기 한도 없음)
                jp      nz,fl_built
                ld      a,1
                ld      (fl_ok),a
                call    il_next
                jp      z,fl_end
                ld      hl,ILBUF
fl_line:        ld      a,(hl)
                cp      ' '
                jr      z,fl_sk
                cp      9
                jr      z,fl_sk
                cp      13
                jr      z,fl_sk
                cp      10
                jr      nz,fl_go
fl_sk:          inc     hl
                jp      fl_line
fl_go:          or      a
                jp      z,fl_skipln
                cp      ';'
                jp      z,fl_skipln
                ld      a,(fl_view)             ; 뷰어 찾기 모드: "VIEW " 로 시작하는 줄만
                or      a
                jr      z,fl_nv
                call    fl_eatv
                jp      nz,fl_skipln
fl_nv:          ld      de,lk_ext               ; 확장자 (대문자, 3 칸 공백 채움)
                ld      a,' '
                ld      (de),a
                inc     de
                ld      (de),a
                inc     de
                ld      (de),a
                ld      de,lk_ext
                ld      b,3
fl_e:           ld      a,(hl)
                cp      '='
                jp      z,fl_eq
                cp      ' '
                jp      z,fl_eq
                or      a
                jp      z,fl_skipln
                cp      13
                jp      z,fl_skipln
                cp      10
                jp      z,fl_skipln
                ld      c,a
                ld      a,b
                or      a
                ld      a,c
                jp      z,fl_skipln             ; 세 글자 넘는 확장자는 건너뜀
                cp      'a'
                jr      c,fl_up
                cp      'z'+1
                jr      nc,fl_up
                and     0DFh
fl_up:          ld      (de),a
                inc     de
                inc     hl
                dec     b
                jr      fl_e
fl_eq:          ld      a,(hl)                  ; 공백 건너뛰고 '=' 확인
                cp      ' '
                jp      nz,fl_eq2
                inc     hl
                jp      fl_eq
fl_eq2:         cp      '='
                jp      nz,fl_skipln
                inc     hl
fl_sp2:         ld      a,(hl)
                cp      ' '
                jr      nz,fl_cmp
                inc     hl
                jp      fl_sp2
fl_cmp:         push    hl                      ; 확장자 비교
                ld      hl,(lk_ep)
                ld      de,lk_ext
                ld      b,3
fl_c3:          ld      a,(de)
                cp      (hl)
                jr      nz,fl_nm
                inc     hl
                inc     de
                djnz    fl_c3
                pop     hl                      ; 맞음: HL = 명령 글 (ILBUF 안)
                push    hl
                call    il_close
                pop     hl
                ret
fl_nm:          pop     hl
fl_skipln:      call    il_next                 ; 다음 줄
                jp      z,fl_end
                ld      hl,ILBUF
                jp      fl_line
fl_end:         call    il_close
fl_built:       ld      a,(fl_view)
                or      a
                jr      z,fl_bi
                ld      hl,0
                ret
fl_bi:          ld      hl,(lk_ep)              ; 기본: COM, BAT
                ld      a,(hl)
                cp      'C'
                jr      nz,fb_bat
                inc     hl
                ld      a,(hl)
                cp      'O'
                jr      nz,fb_no
                inc     hl
                ld      a,(hl)
                cp      'M'
                jr      z,fb_yes
fb_no:          ld      hl,0
                ret
fb_bat:         cp      'B'
                jr      nz,fb_no
                inc     hl
                ld      a,(hl)
                cp      'A'
                jr      nz,fb_no
                inc     hl
                ld      a,(hl)
                cp      'T'
                jr      nz,fb_no
fb_yes:         ld      hl,txt_pct
                ret

; HL = 명령 틀 → CMDBUF (%s 는 RUNPATH, 그 밖의 %글자는 버림)
expand_cmd:     ld      de,CMDBUF
ex_l:           ld      a,(hl)
                or      a
                jr      z,ex_e
                cp      13
                jr      z,ex_e
                cp      10
                jr      z,ex_e
                push    hl
                ld      hl,-(CMDBUF+190)
                add     hl,de
                pop     hl
                jr      c,ex_e
                ld      a,(hl)
                cp      '%'
                jr      nz,ex_lit
                inc     hl
                ld      a,(hl)
                and     0DFh
                cp      'S'
                jr      z,ex_s
                cp      'F'
                jr      nz,ex_skip
                push    hl                      ; %f = 경로에서 확장자를 뺀 것
                ld      hl,RUNPATH
                push    hl
                ld      bc,0
ex_f1:          ld      a,(hl)
                or      a
                jr      z,ex_f2
                cp      '\'
                jr      nz,ex_f1a
                ld      bc,0
                jr      ex_f1n
ex_f1a:         cp      '.'
                jr      nz,ex_f1n
                ld      b,h
                ld      c,l
ex_f1n:         inc     hl
                jr      ex_f1
ex_f2:          pop     hl
ex_f3:          ld      a,(hl)
                or      a
                jr      z,ex_f6
                ld      a,b
                or      c
                jr      z,ex_f5
                ld      a,h
                cp      b
                jr      nz,ex_f5
                ld      a,l
                cp      c
                jr      z,ex_f6
ex_f5:          ld      a,(hl)
                ld      (de),a
                inc     hl
                inc     de
                jr      ex_f3
ex_f6:          pop     hl
                jr      ex_skip
ex_s:           push    hl
                ld      hl,RUNPATH
                call    copy_z
                pop     hl
ex_skip:        inc     hl
                jr      ex_l
ex_lit:         ld      (de),a
                inc     de
                inc     hl
                jr      ex_l
ex_e:           xor     a
                ld      (de),a
                ret

; CMDBUF = "프로그램 인자..." → PROGBUF (프로그램 경로), TAILBUF (0080h 에 넣을 명령줄)
; .BAT 이면 셸(환경 변수 SHELL, 없으면 COMMAND2.COM)에 배치 파일 이름을 인자로 줌
split_cmd:      ld      hl,CMDBUF
sc_s1:          ld      a,(hl)
                cp      ' '
                jr      nz,sc_s2
                inc     hl
                jr      sc_s1
sc_s2:          ld      de,PROGBUF
                ld      b,62
sc_t:           ld      a,(hl)
                or      a
                jr      z,sc_te
                cp      ' '
                jr      z,sc_te
                ld      (de),a
                inc     hl
                inc     de
                djnz    sc_t
sc_te:          xor     a
                ld      (de),a
sc_s3:          ld      a,(hl)
                cp      ' '
                jr      nz,sc_s4
                inc     hl
                jr      sc_s3
sc_s4:          ld      (sc_rest),hl
                ld      hl,0                    ; 마지막 '.' 찾기 (폴더 구분 '\' 뒤에서만)
                ld      (sc_dot),hl
                ld      hl,PROGBUF
sc_f:           ld      a,(hl)
                or      a
                jr      z,sc_fe
                cp      '\'
                jr      nz,sc_f1
                ld      de,0
                ld      (sc_dot),de
                jr      sc_f2
sc_f1:          cp      '.'
                jr      nz,sc_f2
                ld      (sc_dot),hl
sc_f2:          inc     hl
                jr      sc_f
sc_fe:          ld      de,(sc_dot)             ; HL = PROGBUF 끝
                ld      a,d
                or      e
                jr      nz,sc_has
                ex      de,hl                   ; 확장자 없음 → ".COM" 붙임
                ld      hl,txt_dotcom
                call    copy_z
                xor     a
                ld      (de),a
                jr      sc_nobat
sc_has:         ex      de,hl
                inc     hl
                ld      a,(hl)
                and     0DFh
                cp      'B'
                jr      nz,sc_nobat
                inc     hl
                ld      a,(hl)
                and     0DFh
                cp      'A'
                jr      nz,sc_nobat
                inc     hl
                ld      a,(hl)
                and     0DFh
                cp      'T'
                jr      nz,sc_nobat
                ; --- 배치 파일: 인자 = 배치 파일 경로 [인자]
                ld      de,TAILBUF+1
                ld      a,' '
                ld      (de),a
                inc     de
                ld      hl,PROGBUF
                call    copy_z
                ld      hl,(sc_rest)
                ld      a,(hl)
                or      a
                jr      z,sc_b1
                ld      a,' '
                ld      (de),a
                inc     de
                call    copy_z
sc_b1:          push    de
                ld      hl,envn_shell
                ld      de,PROGBUF
                ld      b,62
                ld      c,F_GENV
                call    BDOS
                pop     de
                ld      a,(PROGBUF)
                or      a
                jr      nz,sc_fin
                push    de
                ld      hl,txt_cmd2
                ld      de,PROGBUF
                call    copy_z
                xor     a
                ld      (de),a
                pop     de
                jr      sc_fin
sc_nobat:       ld      hl,(sc_rest)
                ld      a,(hl)
                or      a
                jr      nz,sc_arg
                xor     a
                ld      (TAILBUF),a             ; 인자 없음
                ret
sc_arg:         ld      de,TAILBUF+1
                ld      a,' '
                ld      (de),a
                inc     de
                call    copy_z
sc_fin:         ld      a,13
                ld      (de),a
                ld      hl,-(TAILBUF+1)
                add     hl,de                   ; HL = 글자 수 (앞 공백 포함, CR 제외)
                ld      a,l
                cp      126
                jr      c,sc_len
                ld      a,126
sc_len:         ld      (TAILBUF),a
                ret

; IX = 패널, DE = 쓸 자리 → 경로 '|' 커서항목이름 '|' 를 덧붙임 (DE 는 끝으로)
ss_panel:       push    ix
                pop     hl
                push    de
                ld      de,PS_PATH
                add     hl,de
                pop     de
                call    copy_z
                ld      a,'|'
                ld      (de),a
                inc     de
                ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                jr      z,ss_nn
                push    de
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                call    disp_name
                pop     de
                ld      hl,nmbuf
                call    copy_z
ss_nn:          ld      a,'|'
                ld      (de),a
                inc     de
                ret

cp_setbuf:      ld      hl,(0006h)
                ld      de,-0600h
                add     hl,de
                ld      de,CPBUF
                or      a
                sbc     hl,de
                jr      c,csb_no
                ld      a,h
                and     0FEh
                ld      h,a
                ld      l,0
                ld      (cp_bufsz),hl
                ld      a,h
                cp      8
                jr      c,csb_no
                xor     a
                ret
csb_no:         or      1
                ret

; HL = 글(ASCIIZ), DE = 대상, B = 최대 글자 수 → 복사 (NUL 도 복사됨)
copy_lim:       ld      a,(hl)
                or      a
                ret     z
                cp      80h
                jr      c,cl_1
                inc     hl                      ; (0.12.12) 한글 쌍(80h 이상 두 바이트)은 쪼개지 않음
                ld      a,(hl)
                dec     hl
                cp      80h
                jr      c,cl_1
                ld      a,b
                cp      2
                ret     c
                ld      a,(hl)
                ld      (de),a
                inc     hl
                inc     de
                ld      a,(hl)
                ld      (de),a
                inc     hl
                inc     de
                dec     b
                dec     b
                jr      nz,copy_lim
                ret
cl_1:           ld      a,(hl)
                ld      (de),a
                inc     hl
                inc     de
                djnz    copy_lim
                ret

; ======================================================================
; 폴더 복사 (재귀). 폴더 안의 파일·하위 폴더를 한 단계씩 내려가 처리하고, 패널의 경로를 바꿔 다시 부름.
; 안의 파일은 cp_one 으로 하나씩 복사 (파일 복사 코드 그대로 사용, 폴더는 재귀).
; ======================================================================
; HL = 원본 폴더 항목
copy_folder:    ld      (cp_ent),hl
                ld      a,(lv_depth)
                cp      MAXD
                jr      c,cf_ok
                ld      hl,txt_deep
                jp      cf_fail
cf_ok:          ld      hl,(cp_ent)
                call    disp_name               ; nmbuf = 원본 폴더 이름
                ld      hl,(cp_src)             ; 원본 폴더 전체 경로 → srcpath
                ld      de,PS_PATH
                add     hl,de
                ld      de,srcpath
                call    mkpath
                ld      hl,srcpath
                call    strlen
                cp      52
                jr      c,cf_l1
                ld      hl,txt_plong
                jp      cf_fail
cf_l1:          ld      hl,(cp_dst)             ; 자기 안으로 복사하는가 (대상 경로가 원본 폴더 경로로 시작)
                ld      bc,PS_PATH
                add     hl,bc
                ex      de,hl
                ld      hl,srcpath
                call    path_inside
                jr      nz,cf_l2
                ld      hl,txt_inself
                jp      cf_fail
cf_l2:          ld      hl,(cp_ent)             ; 대상 별칭 = 원본 8.3 이름 (겹치면 바꿈)
                ld      de,cp_alias
                ld      bc,11
                ldir
                xor     a
                ld      (cp_over),a
                ld      ix,(cp_dst)
                ld      de,cp_alias
                call    find_n11
                ld      a,h
                or      l
                jr      z,cf_lfn
                ld      (cp_hit),hl
                ex      de,hl
                ld      hl,(cp_ent)
                call    names_eq
                jr      nz,cf_gen
                jr      cf_chk                  ; 같은 이름의 폴더가 이미 있음
cf_gen:         call    gen_alias
                jr      nz,cf_noal
                jr      cf_make
cf_lfn:         ld      hl,(cp_ent)             ; 8.3 은 다르지만 긴 이름이 같은 항목이 있는가
                ld      de,EN_LFN
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ld      a,d
                or      e
                jr      z,cf_make
                ex      de,hl
                ld      ix,(cp_dst)
                call    find_lfn
                ld      a,h
                or      l
                jr      z,cf_make
                ld      (cp_hit),hl
                ld      de,cp_alias
                ld      bc,11
                ldir
cf_chk:         ld      hl,(cp_hit)             ; 이미 있는 항목이 폴더여야 함
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                jr      nz,cf_exist
                ld      hl,txt_isfile           ; 같은 이름이 파일: 이 폴더는 건너뜀
                ld      (msg_p),hl
                ld      hl,cp_skip
                inc     (hl)
                ret
cf_exist:       ld      a,1                     ; 있는 폴더에 합침 (만들지 않음)
                ld      (cp_over),a
                jr      cf_make
cf_noal:        ld      hl,txt_noalias
                jp      cf_fail
cf_make:        ld      hl,cp_alias             ; 대상 폴더 경로 → dstpath
                call    disp_name
                ld      hl,(cp_dst)
                ld      de,PS_PATH
                add     hl,de
                ld      de,dstpath
                call    mkpath
                ld      hl,dstpath
                call    strlen
                cp      52
                jr      c,cf_l3
                ld      hl,txt_plong
                jp      cf_fail
cf_l3:          ld      a,(cp_over)
                or      a
                jr      nz,cf_rec
                ld      de,dstpath              ; 폴더 만들기 (_CREATE, 속성에 폴더 비트)
                xor     a
                ld      b,10h
                ld      c,F_CREATE
                call    BDOS
                or      a
                jr      z,cf_made
                call    explain_err
                ld      a,1
                ld      (cp_abort),a
                ret
cf_made:        call    lfn_copy                ; 폴더의 긴 이름
cf_rec:         call    push_level
                ld      hl,0
                ld      (cp_i),hl
cf_l:           ld      a,(cp_abort)
                or      a
                jr      nz,cf_done
                ld      ix,(cp_src)
                ld      hl,(cp_i)
                ld      e,(ix+PS_COUNT)
                ld      d,(ix+PS_COUNT+1)
                or      a
                sbc     hl,de
                jr      nc,cf_done
                ld      hl,(cp_i)
                call    ptr_get
                call    is_dotdot
                jr      z,cf_n
                call    cp_one
cf_n:           ld      hl,(cp_i)
                inc     hl
                ld      (cp_i),hl
                jr      cf_l
cf_done:        jp      pop_level
cf_fail:        ld      (msg_p),hl              ; 알림 글 + 복사 전체 중단
                ld      a,1
                ld      (cp_abort),a
                ret

; 한 단계 안으로: 패널의 경로와 반복 번호를 저장하고 경로를 srcpath·dstpath 로 바꿈
push_level:     call    lv_slot                 ; HL = 저장 칸
                ex      de,hl
                ld      hl,(cp_src)
                ld      bc,PS_PATH
                add     hl,bc
                ld      bc,64
                ldir
                ld      hl,(cp_dst)
                ld      bc,PS_PATH
                add     hl,bc
                ld      bc,64
                ldir
                ld      hl,(cp_i)
                ld      a,l
                ld      (de),a
                inc     de
                ld      a,h
                ld      (de),a
                ld      hl,lv_depth
                inc     (hl)
                ld      hl,(cp_src)
                ld      bc,PS_PATH
                add     hl,bc
                ex      de,hl
                ld      hl,srcpath
                call    strcpy_nul
                ld      hl,(cp_dst)
                ld      bc,PS_PATH
                add     hl,bc
                ex      de,hl
                ld      hl,dstpath
                call    strcpy_nul
                ld      ix,(cp_src)
                call    panel_load
                ld      ix,(cp_dst)
                jp      panel_load

; 한 단계 밖으로: 저장해 둔 경로·반복 번호를 되돌리고 패널을 다시 읽음
pop_level:      ld      hl,lv_depth
                dec     (hl)
                call    lv_slot
                ld      de,(cp_src)
                push    hl
                ld      hl,PS_PATH
                add     hl,de
                ex      de,hl
                pop     hl
                ld      bc,64
                ldir
                push    hl
                ld      hl,(cp_dst)
                ld      bc,PS_PATH
                add     hl,bc
                ex      de,hl
                pop     hl
                ld      bc,64
                ldir
                ld      a,(hl)
                ld      (cp_i),a
                inc     hl
                ld      a,(hl)
                ld      (cp_i+1),a
                ld      ix,(cp_src)
                call    panel_load
                ld      ix,(cp_dst)
                jp      panel_load

; HL = LVSTK + lv_depth × LVSZ
lv_slot:        ld      a,(lv_depth)
                ld      b,a
                ld      hl,LVSTK
                or      a
                ret     z
                ld      de,LVSZ
lv_a:           add     hl,de
                djnz    lv_a
                ret

; HL = 글(ASCIIZ) → A = 길이
strlen:         ld      b,0
sl_l:           ld      a,(hl)
                or      a
                jr      z,sl_e
                inc     hl
                inc     b
                jr      sl_l
sl_e:           ld      a,b
                ret

; HL → DE 로 NUL 까지 복사 (NUL 포함)
strcpy_nul:     ld      a,(hl)
                ld      (de),a
                inc     hl
                inc     de
                or      a
                jr      nz,strcpy_nul
                ret

; HL = 기준 경로, DE = 경로 → Z 이면 DE 경로가 HL 경로와 같거나 그 아래
path_inside:    ld      a,(hl)
                or      a
                jr      z,pn_end
                ld      b,a
                ld      a,(de)
                cp      b
                ret     nz
                inc     hl
                inc     de
                jr      path_inside
pn_end:         ld      a,(de)
                or      a
                ret     z
                cp      '\'
                ret

; --- 고른 항목 번호 맵 (selmap, 48 바이트 = 384 비트) ---
selmap_build:   ld      hl,selmap
                ld      de,selmap+1
                ld      (hl),0
                ld      bc,47
                ldir
                ld      ix,(cp_src)
                ld      hl,0
                ld      (sm_i),hl
sb_l:           ld      hl,(sm_i)
                ld      e,(ix+PS_COUNT)
                ld      d,(ix+PS_COUNT+1)
                or      a
                sbc     hl,de
                ret     nc
                ld      hl,(sm_i)
                call    ptr_get
                ld      de,EN_FLAG
                add     hl,de
                bit     0,(hl)
                jr      z,sb_n
                ld      hl,(sm_i)
                call    sm_addr
                or      (hl)
                ld      (hl),a
sb_n:           ld      hl,(sm_i)
                inc     hl
                ld      (sm_i),hl
                jr      sb_l

; HL = 번호 → Z 이면 맵에 없음
selmap_test:    call    sm_addr
                and     (hl)
                ret

; HL = 번호 → HL = 맵의 바이트 번지, A = 비트 마스크
sm_addr:        ld      a,l
                and     7
                ld      b,a
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                ld      de,selmap
                add     hl,de
                ld      a,1
                inc     b
sm_s:           dec     b
                ret     z
                rlca
                jr      sm_s

; BIOS 호출: IX = 번지 (슬롯 간 호출). 아래는 레지스터 보존용
bios:           ld      iy,(EXPTBL)
                call    CALSLT
                ei
                ret

; 주 슬롯 EXPTBL) → HL 번지 → 바이트 읽기
; A  (BIOS RDSLT 와 CALSLT 용)
rd_main:        push    ix
                push    iy
                ld      iy,(EXPTBL)
                ld      a,(EXPTBL+1)
                ld      ix,RDSLT_ADDR
                call    CALSLT
                ei
                pop     iy
                pop     ix
                ret

; VDP 레지스터 B ← A 쓰기
vdp_wreg:       ld      d,a
                ld      a,(vdp_cmd)
                ld      c,a
                di
                out     (c),d
                ld      a,b
                or      80h
                out     (c),a
                ei
                ret

; VRAM 쓰기 주소 = HL (0~3FFFh, R#14 = 0 가정)
scr_vaddr:      ld      a,(vdp_cmd)
                ld      c,a
                di
                out     (c),l
                ld      a,h
                and     3Fh
                or      40h
                out     (c),a
                ei
                ret

; HL = VRAM 주소, DE = 원본, B = 개수 (0 = 256)
scr_write:      ld      a,(s7_on)
                or      a
                jp      nz,s7_write
                push    de
                push    bc
                call    scr_vaddr
                pop     bc
                pop     hl
                ld      a,(vdp_dat)
                ld      c,a
sw_l:           ld      a,(hl)
                out     (c),a
                inc     hl
                djnz    sw_l
                ret

; A = 줄 번호 → HL = 줄 시작 80 (이름표 주소)
row_addr:       ld      l,a
                ld      h,0
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; ×16
                ld      b,h
                ld      c,l
                add     hl,hl
                add     hl,hl                   ; ×64
                add     hl,bc                   ; ×80
                ret

; 깜박임(강조): A = 줄, B = 패널(0/1), C = 줄 FFh 강조, 0 보통. 이 패널 줄만
hl_set:         ld      e,a
                ld      a,(s7_on)
                or      a
                ret     nz                      ; SCREEN 7 에서는 깜박임 표를 쓰지 않음
                ld      d,0
                ld      l,e
                ld      h,d
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; ×8
                add     hl,de
                add     hl,de                   ; ×10
                ld      a,b
                or      a
                jr      z,hs_a
                ld      de,5
                add     hl,de
hs_a:           ld      de,0800h
                add     hl,de
                ld      a,c
                ld      (hl_val),a
                call    scr_vaddr
                ld      a,(vdp_dat)
                ld      c,a
                ld      a,(hl_val)
                ld      b,4
hs_l:           out     (c),a
                djnz    hs_l
                ld      a,(hl_val)
                and     0FEh                    ; 줄의 마지막 구분선 자리는 강조하지 않음
                out     (c),a
                ret

scr_init:       ld      a,(LINL40)
                ld      (old_linl),a
                ld      a,(CSRSW)
                ld      (old_csrsw),a
                ld      a,(SCRMOD)
                ld      (old_scrmod),a
                ld      a,(CNSDFG)
                ld      (old_cnsdfg),a
                call    dos_grab                ; ESC 로 다시 보여 줄 DOS 화면을 VRAM 에 맡겨 둠
                call    scr_text
                jp      si_fk
; 글자 화면(80칸) 만들기 — 처음 시작과 SCREEN 7 에서 돌아올 때 같이 씀
scr_text:       ld      a,80
                ld      (LINL40),a
                ld      a,24
                ld      (CRTCNT),a
                ld      a,15
                ld      (FORCLR),a
                ld      a,4
                ld      (BAKCLR),a
                ld      (BDRCLR),a
                xor     a
                ld      ix,CHGMOD               ; SCREEN 0 (WIDTH 80)
                call    bios
                ld      ix,CHGCLR
                call    bios
                xor     a
                ld      (CSRSW),a               ; 커서 감춤
                ld      ix,ERAFNK
                call    bios
                xor     a
                ld      (CNSDFG),a
                xor     a
                ld      b,14
                call    vdp_wreg                ; R#14 = 0 (VRAM 위쪽 주소)
                ld      a,4Fh
                ld      b,12
                call    vdp_wreg                ; R#12 = 강조 색 (글자 4, 바탕 15)
                ld      a,0F0h
                ld      b,13
                call    vdp_wreg                ; R#13 = 켜짐 15, 꺼짐 0 → 강조 칸만 켜짐, R#12 는 강조 색
                call    clear_screen
                ; 구분선 글자 패턴 (개당 8 바이트, 1000h 부터) 1~3 번에 그림
                ld      hl,1000h+CH_VBAR*8
                call    scr_vaddr
                ld      a,(vdp_dat)
                ld      c,a
                ld      hl,pat_lines
                ld      b,72
sc_3:           ld      a,(hl)
                out     (c),a
                inc     hl
                djnz    sc_3
                ret
si_fk:
                ; 기능키 F1~F10 의 글자 하나(80h~89h)로 바꿔 둠 (시작할 때 원래 글자를 OLDFNK 에 모아 둠)
                ld      hl,FNKSTR
                ld      de,OLDFNK
                ld      bc,160
                ldir
                ld      hl,FNKSTR
                ld      de,fk_codes
                ld      b,10
sc_4:           ld      a,(de)
                inc     de
                ld      (hl),a
                inc     hl
                ld      (hl),0
                push    de
                ld      de,15
                add     hl,de
                pop     de
                djnz    sc_4
                ld      a,(s7_want)
                or      a
                ret     z
                jp      s7_enter

; 이름표는 공백으로, 깜박임(강조)은 0 으로
clear_screen:   ld      a,(s7_on)
                or      a
                jp      nz,s7_clear
                ld      hl,0
                call    scr_vaddr
                ld      a,(vdp_dat)
                ld      c,a
                ld      hl,2000
sc_1:           ld      a,' '
                out     (c),a
                dec     hl
                ld      a,h
                or      l
                jr      nz,sc_1
                ld      hl,0800h
                call    scr_vaddr
                ld      a,(vdp_dat)
                ld      c,a
                ld      hl,270
sc_2:           xor     a
                out     (c),a
                dec     hl
                ld      a,h
                or      l
                jr      nz,sc_2
                ret

; 글자 화면의 이름표(글자 칸)를 공백으로: 80 칸이면 2160 바이트, 40 칸이면 960 (그 뒤는 글꼴이라 건드리지 않음)
clr_nt:         ld      a,(old_scrmod)
                or      a
                ret     nz
                ld      hl,0
                call    scr_vaddr
                ld      a,(vdp_dat)
                ld      c,a
                ld      hl,960
                ld      a,(old_linl)
                cp      41
                jr      c,cn_l
                ld      hl,2160
cn_l:           ld      a,' '
                out     (c),a
                dec     hl
                ld      a,h
                or      l
                jr      nz,cn_l
                ret

scr_exit:       call    hgl_close
                ld      hl,OLDFNK              ; 기능키 글자 되돌림
                ld      de,FNKSTR
                ld      bc,160
                ldir
                xor     a
                ld      b,12
                call    vdp_wreg
                xor     a
                ld      b,13
                call    vdp_wreg
                ld      a,(old_linl)
                ld      (LINL40),a
                ld      a,(old_csrsw)
                ld      (CSRSW),a
                ld      a,(dos_rest)            ; (끝낼 때만) 처음 DOS 화면을 되돌림
                or      a
                call    nz,dos_fetch
                call    s7_leave
                ld      a,(old_scrmod)
                ld      ix,CHGMOD
                call    bios
                call    clr_nt                  ; X86FC 가 그린 글자를 다 지움
                ld      a,(dos_rest)
                or      a
                call    nz,dos_wr
                ld      a,(old_cnsdfg)
                ld      (CNSDFG),a
                or      a
                ret     z
                ld      ix,DSPFNK
                jp      bios

; ======================================================================
; 고르기(표시): 항목 EN_FLAG 비트 0
; ======================================================================
; HL = 항목 → Z 이면 ".." (고를 수 없음)
is_dotdot:      ld      a,(hl)
                cp      '.'
                jr      nz,idd_n
                inc     hl
                ld      a,(hl)
                dec     hl
                cp      '.'
                ret
idd_n:          or      1
                ret

; IX = 패널: 커서 항목을 고르기·해제하고 한 줄 아래로
sel_toggle:     ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                ret     z
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                call    is_dotdot
                jr      z,sel_dn                ; ".." 는 고르지 않음 (그냥 아래로)
                ld      de,EN_FLAG
                add     hl,de
                ld      a,(hl)
                xor     1
                ld      (hl),a
                ld      l,(ix+PS_SEL)
                ld      h,(ix+PS_SEL+1)
                and     1
                jr      z,st_off
                inc     hl
                jr      st_set
st_off:         dec     hl
st_set:         ld      (ix+PS_SEL),l
                ld      (ix+PS_SEL+1),h
sel_dn:         jp      move_down

; IX = 패널: 하나라도 안 고른 항목이 있으면 모두 고름, 모두 골랐으면 모두 해제 (".." 제외)
sel_all:        ld      l,(ix+PS_COUNT)
                ld      h,(ix+PS_COUNT+1)
                ld      a,h
                or      l
                ret     z
                ld      (sa_n),hl
                xor     a
                ld      (sa_any),a
                ld      hl,0
                ld      (sa_i),hl
sa_p1:          ld      hl,(sa_i)
                ld      de,(sa_n)
                or      a
                sbc     hl,de
                jr      nc,sa_p1e
                ld      hl,(sa_i)
                call    ptr_get
                call    is_dotdot
                jr      z,sa_n1
                ld      de,EN_FLAG
                add     hl,de
                bit     0,(hl)
                jr      nz,sa_n1
                ld      a,1
                ld      (sa_any),a
sa_n1:          ld      hl,(sa_i)
                inc     hl
                ld      (sa_i),hl
                jr      sa_p1
sa_p1e:         ld      hl,0
                ld      (sa_i),hl
                ld      (sa_cnt),hl
sa_p2:          ld      hl,(sa_i)
                ld      de,(sa_n)
                or      a
                sbc     hl,de
                jr      nc,sa_p2e
                ld      hl,(sa_i)
                call    ptr_get
                call    is_dotdot
                jr      z,sa_n2
                ld      de,EN_FLAG
                add     hl,de
                ld      a,(sa_any)
                ld      (hl),a
                or      a
                jr      z,sa_n2
                ld      hl,(sa_cnt)
                inc     hl
                ld      (sa_cnt),hl
sa_n2:          ld      hl,(sa_i)
                inc     hl
                ld      (sa_i),hl
                jr      sa_p2
sa_p2e:         ld      hl,(sa_cnt)
                ld      (ix+PS_SEL),l
                ld      (ix+PS_SEL+1),h
                ret

; 모두 다시 그림
draw_all:       ld      a,(s7_ovb)              ; (0.12.21) 오버레이 실행 중이 아니면 얇은 선 배치로 그림
                or      a
                jr      nz,da_o
                ld      a,(s7_thm)
                ld      (s7_th),a
da_o:           ld      a,(s7_dirty)
                or      a
                call    nz,s7_undirty
                ld      a,(pop_y1)              ; (0.12.33) 팝업이 덮은 줄만 해시를 비워 그 줄만 다시 그림
                or      a
                call    nz,s7_popinv
                call    draw_fk                 ; 기능키 안내 (평소 F1~F5, Shift 를 누르면 F6~F10)
                ld      hl,hbuf                ; 기능키 안내 줄과 가로선 (전체 폭, 세로선이 만나는 39 열 포함)
                ld      de,hbuf+1
                ld      (hl),CH_HBAR
                ld      bc,79
                ldir
                ld      a,(v_rbot)
                call    row_addr
                ld      de,hbuf
                ld      b,79
                call    scr_write
                call    draw_divider
                xor     a
                call    panel_ix
                call    draw_panel
                ld      a,1
                call    panel_ix
                call    draw_panel
                call    draw_status
                jp      s7_bands

; 세로 구분선 (열 39, 줄 0~22). 가로선(줄 1, 20)과 만나는 곳은 ┼ 모양
draw_divider:   ld      a,(s7_th)               ; 얇은 선 배치는 s7_bands 가 세로선도 그림
                or      a
                ret     nz
                xor     a
                ld      (dv_r),a
dv_l:           ld      a,(dv_r)
                call    row_addr
                ld      de,39
                add     hl,de
                ld      a,CH_VBAR
                ld      (dv_c),a
                ld      a,(dv_r)
                cp      1
                jr      z,dv_x
                ld      b,a
                ld      a,(v_rrule)
                cp      b
                jr      nz,dv_w
                ld      a,CH_TUP                ; 아래 가로선과 만나는 곳: 위로만 이어짐 (아래는 한 줄 정보 줄)
                ld      (dv_c),a
                jr      dv_w
dv_x:           ld      a,CH_CROSS
                ld      (dv_c),a
dv_w:           ld      de,dv_c
                ld      b,1
                call    scr_write
                ld      a,(dv_r)
                inc     a
                ld      (dv_r),a
                ld      hl,v_rrule              ; 세로 구분선은 목록 아래 가로선까지
                cp      (hl)
                jr      c,dv_l
                jr      z,dv_l
                ret

blank_line:     ld      hl,linebuf
                ld      de,linebuf+1
                ld      (hl),' '
                ld      bc,39
                ldir
                ret

; linebuf(40) 를 A 줄 패널 pn 칸에
put_row:        call    row_addr
                ld      a,(pn)
                or      a
                jr      z,pr_a
                ld      de,40
                add     hl,de
pr_a:           ld      de,linebuf
                ld      b,PANW
                call    scr_write
                xor     a
                ld      (s7_pair),a             ; (SCREEN 7) 줄 색은 한 줄만 씀
                ret

; A 줄에 가로선 (패널 열 39 포함)
hline:          push    af
                ld      hl,linebuf
                ld      de,linebuf+1
                ld      (hl),CH_HBAR
                ld      bc,PANW-1
                ldir
                pop     af
                jp      put_row

; A 줄에 강조 C(FFh/0) 를 (패널 pn)
hl_row:         ld      hl,pn
                ld      b,(hl)
                jp      hl_set

; IX = 패널 그리기: 머리(경로), 목록 20 줄, 이름 줄 ...
draw_panel:     push    ix
                pop     hl
                ld      de,pstate
                or      a
                sbc     hl,de
                ld      a,h
                or      l
                ld      a,0
                jr      z,dp_pn
                ld      a,1
dp_pn:          ld      (pn),a
                ld      b,a
                ld      a,(act)
                cp      b
                ld      a,0
                jr      nz,dp_ia
                ld      a,1
dp_ia:          ld      (isact),a
                ; --- 머리: 경로 (38 글자, 길면 뒤쪽) ---
                call    blank_line
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      b,0
                push    hl
dp_len:         ld      a,(hl)
                or      a
                jr      z,dp_len_e
                inc     hl
                inc     b
                jr      dp_len
dp_len_e:       pop     hl
                ld      a,b
                cp      38
                jr      c,dp_cpy
                sub     38
                ld      e,a
                ld      d,0
                add     hl,de
                ld      b,38
dp_cpy:         ld      a,b
                or      a
                jr      z,dp_hdr
                ld      de,linebuf+1
                ld      c,b
                ld      b,0
                ldir
dp_hdr:         xor     a
                call    put_row
                xor     a                       ; 머리 줄은 강조하지 않음 (커서 줄만 강조)
                ld      c,0
                call    hl_row
                ld      a,1                     ; 경로 줄 아래 가로선
                call    hline
                ; --- 목록 ---
                call    s7_lscr                 ; (0.12.9) 목록이 밀렸으면 VRAM 을 밀어 새 줄만 그리게
                xor     a
                ld      (dp_i),a
dp_row:         ld      a,(dp_i)
                ld      hl,v_lrows
                cp      (hl)
                jp      nc,dp_rows_done
                ld      l,(ix+PS_TOP)
                ld      h,(ix+PS_TOP+1)
                ld      e,a
                ld      d,0
                add     hl,de
                ld      (dp_idx),hl
                ld      e,(ix+PS_COUNT)
                ld      d,(ix+PS_COUNT+1)
                or      a
                sbc     hl,de
                jr      nc,dp_blank
                ld      hl,(dp_idx)
                call    ptr_get
                call    fmt_line
                ld      a,1
                ld      (s7_ent),a
                ld      hl,(en_p)              ; 고른 항목이면 '*'
                ld      de,EN_FLAG
                add     hl,de
                bit     0,(hl)
                jr      z,dp_nosel
                ld      a,'*'
                ld      (linebuf),a
dp_nosel:       ld      hl,(dp_idx)
                ld      e,(ix+PS_CUR)
                ld      d,(ix+PS_CUR+1)
                or      a
                sbc     hl,de
                jr      nz,dp_put
                ld      a,(isact)
                or      a
                jr      z,dp_put
                ld      a,(linebuf)             ; 커서 줄('*' 가 없으면 그 자리에 '>')
                cp      '*'
                jr      z,dp_cur
                ld      a,'>'
                ld      (linebuf),a
dp_cur:         ld      c,0FFh
                jr      dp_put2
dp_blank:       xor     a
                ld      (s7_ent),a
                call    blank_line
dp_put:         ld      c,0
dp_put2:        call    s7_prep
                ld      a,(dp_i)
                add     a,LIST_Y
                push    bc
                call    put_row
                pop     bc
                ld      a,(dp_i)
                add     a,LIST_Y
                call    hl_row
                ld      a,(dp_one)
                or      a
                ret     nz                      ; 한 줄만 그리는 중
                ld      a,(dp_i)
                inc     a
                ld      (dp_i),a
                jp      dp_row
                ; --- 가로선 (목록 아래, 긴 이름 줄 위) ---
dp_rows_done:   ld      a,(v_rrule)
                jp      hline

; HL = 번호 → HL = 정렬된 항목 주소 (IX = 패널)
ptr_get:        add     hl,hl
                ld      e,(ix+PS_PTR)
                ld      d,(ix+PS_PTR+1)
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ex      de,hl
                ret

; HL = 항목 → nmbuf = "NAME.EXT" (ASCIIZ)
disp_name:      ld      (dn_e),hl
                ld      de,nmbuf
                ld      b,8
dn_1:           ld      a,(hl)
                cp      ' '
                jr      z,dn_2
                ld      (de),a
                inc     hl
                inc     de
                djnz    dn_1
dn_2:           ld      hl,(dn_e)
                ld      bc,EN_EXT
                add     hl,bc
                ld      a,(hl)
                cp      ' '
                jr      z,dn_end
                ld      a,'.'
                ld      (de),a
                inc     de
                ld      b,3
dn_3:           ld      a,(hl)
                cp      ' '
                jr      z,dn_end
                ld      (de),a
                inc     hl
                inc     de
                djnz    dn_3
dn_end:         xor     a
                ld      (de),a
                ret

; HL = 항목 → nmbuf = 찾기·정렬용 이름: 긴 이름이 있으면 그것 (대문자로, 15 글자 ...)
search_name:    push    hl
                ld      de,EN_LFN
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                pop     hl
                ld      a,d
                or      e
                jp      z,disp_name
                ex      de,hl
                ld      de,nmbuf
                ld      b,15
sn_l:           ld      a,(hl)
                or      a
                jr      z,sn_e
                cp      'a'
                jr      c,sn_s
                cp      'z'+1
                jr      nc,sn_s
                and     0DFh
sn_s:           ld      (de),a
                inc     hl
                inc     de
                djnz    sn_l
sn_e:           xor     a
                ld      (de),a
                ret

; HL = 원본(ASCIIZ), DE = 대상. NUL 은 복사하지 않음. DE 를 돌려줌
copy_z:         ld      a,(hl)
                or      a
                ret     z
                ld      (de),a
                inc     hl
                inc     de
                jr      copy_z

; HL = 항목 → linebuf (40 칸: 이름 · 확장자 · 크기(또는 <DIR>) · 날짜
; en_p 항목에 긴 이름이 있으면 linebuf+1 부터 15 칸에 그 이름 (넘치면 14 글자 + '~'), HL 보존
fl_long:        push    hl
                ld      hl,(en_p)
                ld      de,EN_LFN
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ld      a,d
                or      e
                jr      z,fll_x
                ex      de,hl
                ld      de,linebuf+1
                ld      b,15
                ld      a,(s7_hm)
                or      a
                jr      z,fll_c
                inc     b
fll_c:          ld      a,(hl)
                or      a
                jr      z,fll_p
                ld      (de),a
                inc     hl
                inc     de
                djnz    fll_c
                ld      a,(hl)
                or      a
                jr      z,fll_x
                dec     de
                ld      a,'~'
                ld      (de),a
                jr      fll_x
fll_p:          ld      a,' '
fll_p1:         ld      (de),a
                inc     de
                djnz    fll_p1
fll_x:          pop     hl
                ret
fmt_line:       ld      (en_p),hl
                call    blank_line
                ld      hl,(en_p)
                ld      de,linebuf+1
                ld      bc,8
                ldir
                ld      de,linebuf+10
                ld      bc,3
                ldir
                ; call    fl_long                 ; 긴 이름이 있으면 이름·확장자 칸(15칸)에 그것을 보임
                ld      a,(hl)                  ; 속성
                and     10h
                jr      z,fl_file
                ld      hl,txt_dir
                ld      de,linebuf+22
                ld      bc,5
                ldir
                jr      fl_date
fl_file:        ld      hl,(en_p)
                ld      de,EN_SIZE
                add     hl,de
                ld      de,linebuf+17
                call    dec32
fl_date:        ld      hl,(en_p)
                ld      de,EN_DATE
                add     hl,de
                ld      c,(hl)
                inc     hl
                ld      b,(hl)
                ld      a,b
                or      c
                ret     z                       ; 날짜 없음
                ld      de,linebuf+30
                ld      a,b
                srl     a
                add     a,80
fl_y1:          cp      100
                jr      c,fl_y2
                sub     100
                jr      fl_y1
fl_y2:          call    put2
                ld      a,'-'
                ld      (de),a
                inc     de
                ld      a,c
                rlca
                rlca
                rlca
                and     7
                ld      l,a
                ld      a,b
                and     1
                rlca
                rlca
                rlca
                or      l
                call    put2
                ld      a,'-'
                ld      (de),a
                inc     de
                ld      a,c
                and     1Fh
                jp      put2                    ; 시간(hh:mm)은 표시하지 않음 (날짜만)

; HL = 값(16 비트) → 앞 공백으로 맞춰 DE 에 쓰고 DE 를 진행
put_dec16:      push    de
                ld      (cnt32),hl
                ld      hl,0
                ld      (cnt32+2),hl
                ld      hl,cnt32
                ld      de,tmp10
                call    dec32
                pop     de
                ld      hl,tmp10
pd_s:           ld      a,(hl)
                cp      ' '
                jr      nz,pd_c
                inc     hl
                jr      pd_s
pd_c:           ld      b,10
pd_l:           ld      a,(hl)
                cp      ' '
                ret     z
                or      a
                ret     z
                ld      (de),a
                inc     hl
                inc     de
                djnz    pd_l
                ret

; A (0~99) → 두 자리로 DE 에 쓰고 DE 를 진행 (BC 보존)
put2:           push    bc
                ld      c,0
p2_l:           cp      10
                jr      c,p2_e
                sub     10
                inc     c
                jr      p2_l
p2_e:           ld      b,a
                ld      a,c
                add     a,'0'
                ld      (de),a
                inc     de
                ld      a,b
                add     a,'0'
                ld      (de),a
                inc     de
                pop     bc
                ret

; HL = 4 바이트 값 (낮은 것 먼저), DE = 10 칸 → 오른쪽에 맞춰 씀 (앞은 공백)
dec32:          push    de
                ld      de,v32
                ld      bc,4
                ldir
                pop     de
                ld      hl,p10tab
                ld      a,10
                ld      (dg_cnt),a
                ld      a,1
                ld      (dg_lead),a
dg_next:        push    de
                ld      de,d_pw
                ld      bc,4
                ldir
                pop     de
                push    hl                      ; 앞의 다음 자리
                push    de                      ; 대상
                ld      b,0
dg_sub:         ld      hl,(v32)
                ld      de,(d_pw)
                or      a
                sbc     hl,de
                push    hl
                ld      hl,(v32+2)
                ld      de,(d_pw+2)
                sbc     hl,de
                jr      c,dg_under
                ld      (v32+2),hl
                pop     hl
                ld      (v32),hl
                inc     b
                jr      dg_sub
dg_under:       pop     hl
                pop     de
                ld      c,b
                ld      a,(dg_cnt)
                cp      1
                jr      z,dg_show
                ld      a,c
                or      a
                jr      nz,dg_show
                ld      a,(dg_lead)
                or      a
                jr      z,dg_show
                ld      a,' '
                jr      dg_put
dg_show:        xor     a
                ld      (dg_lead),a
                ld      a,c
                add     a,'0'
dg_put:         ld      (de),a
                inc     de
                pop     hl
                ld      a,(dg_cnt)
                dec     a
                ld      (dg_cnt),a
                jr      nz,dg_next
                ret

; 상태 줄 (줄 22): 패널마다 항목 수, 입력 글자, 알림 글
draw_status:    push    ix
                ld      hl,hbuf                 ; 80 칸 한 줄을 공백으로
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                call    cur_ix
                ld      hl,(msg_p)              ; 알림 글이 있으면 그것
                ld      a,h
                or      l
                jr      z,ds_nm
                ld      de,hbuf+3
                ld      b,56
ds_m:           ld      a,(hl)
                or      a
                jr      z,ds_cn
                ld      (de),a
                inc     hl
                inc     de
                djnz    ds_m
                jr      ds_cn
ds_nm:          ld      a,(typelen)             ; 앞글자 찾기 중이면 입력한 글자
                or      a
                jr      z,ds_nm2
                ld      hl,txt_find
                ld      de,hbuf+3
                call    copy_z
                ld      a,(typelen)
                ld      c,a
                ld      b,0
                ld      hl,typebuf
                ldir
                jr      ds_cn
ds_nm2:         ld      a,(ix+PS_COUNT)         ; 아니면 커서 파일의 긴 이름 (없으면 8.3 이름)
                or      (ix+PS_COUNT+1)
                jr      z,ds_cn
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                push    hl
                ld      de,EN_LFN
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                pop     hl
                ld      a,d
                or      e
                jr      z,ds_83
                ex      de,hl
                ld      de,hbuf+3
                ld      b,56
ds_lc:          ld      a,(hl)
                or      a
                jr      z,ds_cn
                ld      (de),a
                inc     hl
                inc     de
                djnz    ds_lc
                jr      ds_cn
ds_83:          call    disp_name
                ld      hl,nmbuf
                ld      de,hbuf+3
                call    copy_z
ds_cn:          ld      l,(ix+PS_COUNT)         ; 줄 뒤쪽(60 칸부터): 개수
                ld      h,(ix+PS_COUNT+1)
                ld      (cnt32),hl
                ld      hl,0
                ld      (cnt32+2),hl
                ld      hl,cnt32
                ld      de,tmp10
                call    dec32
                ld      hl,tmp10
ds_trim:        ld      a,(hl)
                cp      ' '
                jr      nz,ds_t2
                inc     hl
                jr      ds_trim
ds_t2:          ld      de,hbuf+60
                ld      b,10
ds_t3:          ld      a,(hl)
                or      a
                jr      z,ds_t4
                cp      ' '
                jr      z,ds_t4
                ld      (de),a
                inc     hl
                inc     de
                djnz    ds_t3
ds_t4:          ld      hl,txt_items
                call    copy_z
                ld      l,(ix+PS_SEL)           ; 고른 게 있으면 ", N sel"
                ld      h,(ix+PS_SEL+1)
                ld      a,h
                or      l
                jr      z,ds_out
                ld      a,','
                ld      (de),a
                inc     de
                ld      a,' '
                ld      (de),a
                inc     de
                call    put_dec16
                ld      hl,txt_sel
                call    copy_z
ds_out:         ld      hl,hbuf                 ; 맨 앞 두 칸: 선택된 창 (<- 왼쪽, -> 오른쪽), 한 칸 띄고 이름
                ld      a,(s7_on)
                or      a
                jr      z,ds_t1
                ld      (hl),' '                ; SCREEN 7 은 아래에서 아이콘을 그림
                inc     hl
                ld      (hl),' '
                jr      ds_mw
ds_t1:          ld      a,(act)                 ; (0.12.13) 왼쪽 패널 = 줄 맨 왼쪽 <-, 오른쪽 패널 = 줄 맨 오른쪽 ->
                or      a
                jr      nz,ds_tr
                ld      (hl),'<'
                inc     hl
                ld      (hl),'-'
                jr      ds_mw
ds_tr:          ld      hl,hbuf+77              ; (0.12.22) 글자 화면: 오른쪽 끝 = 선·목록의 마지막 칸(78)
                ld      (hl),'-'
                inc     hl
                ld      (hl),'>'
ds_mw:          ld      a,(s7_on)               ; (0.12.26) SCREEN 7: 활성 패널이 바뀌었으면 정보 줄 해시를 무효로 해서
                or      a                       ; 줄 전체(이전 아이콘이 걸친 칸 77~79·0~1 포함)를 바탕으로 다시 그림
                jr      z,ds_mw2
                ld      a,(act)
                ld      hl,ds_la
                cp      (hl)
                jr      z,ds_mw2
                ld      (hl),a
                ld      a,(v_rstat)
                add     a,a
                add     a,a
                ld      e,a
                ld      d,0
                ld      hl,s7_hash
                add     hl,de
                ld      b,4
ds_mwh:         ld      (hl),0FFh
                inc     hl
                djnz    ds_mwh
ds_mw2:         ld      a,(v_rstat)
                call    row_addr
                ld      de,hbuf
                ld      b,80
                call    scr_write
                ld      a,(s7_on)               ; SCREEN 7: 8x8 화살표 아이콘(주황) 한 칸 (오른쪽 2점은 빈 칸으로 넘침)
                or      a
                jr      z,ds_x
                ld      a,(act)
                add     a,19h
                ld      (ds_ic),a
                ld      a,1
                ld      (s7_pair),a
                ld      a,(v_rstat)
                call    row_addr
                ld      a,(act)                 ; 오른쪽 패널이면 맨 오른쪽 칸(79)에 아이콘
                or      a
                jr      z,ds_ia
                ld      de,78                   ; (0.12.22) 아이콘 8점 폭: 칸 78 에서 2점 왼쪽 = 오른쪽 끝이 선 끝(x 490)과 같음
                add     hl,de
                ld      de,-2
                ld      (s7_xo),de
ds_ia:          ld      de,ds_ic
                ld      b,1
                call    scr_write
                ld      hl,0
                ld      (s7_xo),hl
                xor     a
                ld      (s7_pair),a
ds_x:           call    s7_bdinfo               ; (0.12.28) 정보 줄 글이 칸 79 의 오른쪽 테두리선을 지웠을 수 있어 되살림
                pop     ix
                ret
ds_ic:          db      0
ds_la:          db      0FFh                    ; 마지막으로 그린 활성 패널

; F1 안내 화면: 아무 키나 누르면 돌아감
show_help:      ld      a,3                     ; (0.12.9) 도움말은 오버레이 3 (ov_open.inc)
                ld      b,2
                jp      ov_go

; 키 하나 기다림 → A 에 (BIOS 커서를 지우지 않으려고 CHSNS 로 기다린 뒤 CHGET)
wait_key:       ld      ix,CHSNS
                call    bios
                jr      z,wait_key
                ld      ix,CHGET
                jp      bios

; 글 출력 (DOS 콘솔, 화면을 바꾸지 않고 오류 알림용)
puts:           ld      a,(hl)
                or      a
                ret     z
                push    hl
                ld      e,a
                ld      c,F_CONOUT
                call    BDOS
                pop     hl
                inc     hl
                jr      puts

; ======================================================================
; 글 (안내·오류 메시지)
; ======================================================================
txt_need2:      db      "X86FC needs MSX-DOS2 or Nextor.",13,10,0
txt_needmsx2:   db      "X86FC needs MSX2 or later.",13,10,0
txt_nomem:      db      "X86FC: not enough memory.",13,10,0
txt_star:       db      "*.*",0
txt_dir:        db      "<DIR>"
txt_mv:         db      "Move ",0
txt_moved:      db      "Moved ",0
txt_mvself:     db      "Cannot move a folder into itself",0
txt_mvkeep:     db      "Some items skipped: source folder kept",0
txt_mkq:        db      "New folder name",0
txt_rnq:        db      "Rename to",0
txt_exists:     db      "Name already exists",0
txt_badch:      db      5Ch,'/',':','*','?',22h,'<','>','|'     ; 이름에 쓸 수 없는 글자
txt_sym83:      db      "!#$%&'()-@^_`{}~"                      ; 8.3 이름에 쓸 수 있는 기호
txt_del:        db      "Delete ",0
txt_yn:         db      "?",0
txt_ppyna:      db      "[Y] Yes   [N] No   [A] All   [Esc] Stop",0
txt_items2:     db      " item(s)",0
txt_ro:         db      "Read-only ",0
txt_ro2:        db      ": delete?",0
txt_dqfull:     db      "Too many folders selected",0
txt_deleted:    db      "Deleted ",0
txt_deep:       db      "Folders nested too deep",0
txt_plong:      db      "Path too long",0
txt_inself:     db      "Cannot copy a folder into itself",0
txt_isfile:     db      "A file has that name: folder skipped",0
txt_nocap:      db      "LFN: source pieces not found",0
txt_noalias:    db      "Cannot make a short name",0
txt_filebase:   db      "FILE"
txt_nolfn:      db      "Long name not copied (no room)",0
txt_same:       db      "Same folder in both panels",0
txt_nomem2:     db      "Not enough memory to copy",0
txt_copied:     db      "Copied ",0
txt_files:      db      " file(s)",0
txt_over:       db      "Overwrite ",0
txt_over2:      db      "?",0
txt_items:      db      " items",0
txt_sel:        db      " sel",0
txt_find:       db      "Find: ",0
txt_toolong:    db      "Path too long",0
txt_norun:      db      "No run link for this file type (see X86FC.INI)",0
txt_selfdef:    db      "X86FC.COM",0
envn_prog:      db      "PROGRAM",0
envn_x86:       db      "X86FC",0
rs_empty:       db      0
txt_pct:        db      "%s",0
txt_cmdq:       db      "Command line",0
txt_inh:        db      "[Enter] OK        [Esc] Cancel",0
txt_f5q:        db      "Go to MSX-BASIC?",0
hook_sh:        db      0
sort_mode:      db      0
cmd_ins:        db      0
sort_msgs:      dw      sm0,sm1,sm2,sm3
sm0:            db      "Sort: name",0
sm1:            db      "Sort: extension",0
sm2:            db      "Sort: size (largest first)",0
sm3:            db      "Sort: date (newest first)",0
txt_runbat:     db      "X86FCR1.BAT",0         ; 7 번째 글자(1/2)를 번갈아 씀
txt_pause:      db      "PAUSE",13,10,0
envn_xb:        db      "X86FC_B",0
rg_len:         dw      0
txt_notmp:      db      "Cannot write X86FCR1.BAT / X86FCR2.BAT",0
txt_qdt:        db      " Quick-Dir ",0
txt_qdh:        db      "Up/Dn+Enter 0-9/A-J go  INS add folder  DEL remove  ESC close",0
qd_pa:          db      0
qd_y0:          db      0
qd_nl:          db      0
txt_qdnone:     db      "(empty - press INS to add the current folder)",0
txt_qdfull:     db      "Quick-Dir is full (20)",0
txt_qdrm:       db      "Remove which one? (press its key)",0
envn_xt:        db      "X86FC_T",0
il_raw:         db      0
cl_tmp:         db      0
qd_n:           db      0
qd_i:           db      0
qd_r:           db      0
qd_tmp:         ds      80
txt_dotcom:     db      ".COM",0
txt_cmd2:       db      "COMMAND2.COM",0
envn_shell:     db      "SHELL",0
lk_ep:          dw      0
lk_ext:         ds      3
sc_rest:        dw      0
sc_dot:         dw      0
rs_p0:          dw      0
rs_n0:          dw      0
rs_p1:          dw      0
rs_n1:          dw      0
; 기능키 안내 (줄 23, 기능키 8 개. F1~F5 는 나중에 설정 파일(X86FC.INI)로
txt_fkeys:      db      "1Help           2View           3Edit           4Screen         5Basic          "
txt_fkeys2:     db      "6Rename         7MkDir          8Sort           9Drive          10Exit          "
shift_st:       db      0
; 기능키 안내 줄: Shift 를 누르고 있으면 F6~F10, 아니면 F1~F5
draw_fk:        ld      a,(v_rfkey)
                call    row_addr
                ld      de,txt_fkeys
                ld      a,(shift_st)
                or      a
                jr      z,dfk_n
                ld      de,txt_fkeys2
dfk_n:          ld      b,80
                jp      scr_write
; Shift 가 눌렸다 떼어지면 안내 줄을 바꿈 (키 기다리는 동안 계속 봄).
; (0.12.35) 실기에서 Shift 를 누르고 있는데 약 3초 뒤 F1~F5 로 돌아가던 것 대비: SNSMAT 값과 BIOS 키 인터럽트가 갱신하는
;  NEWKEY 표(FBE5h+6)를 모두 보아 하나라도 "눌림"이면 눌림으로 보고, "뗌"은 3 jiffy(1/20초) 이상 계속 뗀 것으로 읽혔을 때만 인정
fk_shift:       ld      a,(0FBEBh)              ; NEWKEY 행 6
                ld      b,a
                ld      a,6
                ld      ix,SNSMAT
                call    bios
                and     b                       ; 둘 중 하나라도 0 이면 bit 0 = 0 (눌림)
                and     1
                jr      nz,fks_rel
                ld      hl,(JIFFY)
                ld      (fk_t),hl               ; 마지막으로 눌린 것을 본 때
                ld      a,1
                jr      fks_set
fks_rel:        ld      hl,(JIFFY)
                ld      de,(fk_t)
                or      a
                sbc     hl,de
                ld      a,h
                or      a
                ld      a,0
                jr      nz,fks_set              ; 오래전에 눌림 → 뗌
                ld      a,l
                cp      3
                ld      a,0
                jr      nc,fks_set
                ld      a,(shift_st)            ; 잠깐 뗀 것으로 읽힘: 그대로 둠
fks_set:        ld      hl,shift_st
                cp      (hl)
                ret     z
                ld      (hl),a
                jp      draw_fk
fk_t:           dw      0
fk_codes:       db      K_F1,K_F2,K_F3,K_F4,K_F5,K_F6,K_F7,K_F8,K_F9,K_F10   ; F9 는 동작 없음 (글자 안 나옴, ^W 와 겹치지 않게)
; 직접 만든 글자 모양 (한 글자 8 줄, 왼쪽 6 비트). (0.12.25) 1점 가는 선: 세로선은 왼쪽에서 셋째 점(20h),
; 가로선은 다섯째 줄(FCh = 칸 폭 전체). 선이 칸 경계까지 닿아서 이웃 칸과 이어지고, 교차부에 틈이 없음.
pat_lines:      db      20h,20h,20h,20h,20h,20h,20h,20h        ; 01 │
                db      00h,00h,00h,00h,0FCh,00h,00h,00h       ; 02 ─
                db      20h,20h,20h,20h,0FCh,20h,20h,20h       ; 03 ┼
                db      20h,20h,20h,20h,0FCh,00h,00h,00h       ; 04 ┴
                db      00h,00h,00h,00h,3Ch,20h,20h,20h        ; 05 ┌
                db      00h,00h,00h,00h,0E0h,20h,20h,20h       ; 06 ┐
                db      20h,20h,20h,20h,3Ch,00h,00h,00h        ; 07 └
                db      20h,20h,20h,20h,0E0h,00h,00h,00h       ; 08 ┘
                db      20h,20h,20h,20h,3Ch,20h,20h,20h        ; 09 ├
gaps:           dw      1093,364,121,40,13,4,1,0
p10tab:         dw      0CA00h,3B9Ah            ; 1,000,000,000
                dw      0E100h,05F5h            ; 100,000,000
                dw      9680h,0098h             ; 10,000,000
                dw      4240h,000Fh             ; 1,000,000
                dw      86A0h,0001h             ; 100,000
                dw      2710h,0000h             ; 10,000
                dw      03E8h,0000h             ; 1,000
                dw      0064h,0000h             ; 100
                dw      000Ah,0000h             ; 10
                dw      0001h,0000h             ; 1

; ======================================================================
; 변수 (초기값 0)
; ======================================================================
vdp_dat:        db      0
vdp_cmd:        db      0
msx_ver:        db      0
cur_drive:      db      0
act:            db      0
pn:             db      0
isact:          db      0
typelen:        db      0
typebuf:        ds      16
type_t:         dw      0
msg_p:          dw      0
fp_n:           dw      0
fp_i:           dw      0
dir_err:        db      0
cnt_tmp:        dw      0
cp_src:         dw      0
cp_dst:         dw      0
cp_ent:         dw      0
cp_i:           dw      0
cp_cnt:         dw      0
cp_bufsz:       dw      0
cd_n:           dw      0
fn_nm:          dw      0
cp_abort:       db      0
; --- 상태 ---
mv_same:        db      0
cp_skip:        db      0
mv_n:           dw      0
mv_c0:          dw      0
mv_nm:          ds      11
; --- 이름 입력·폴더 만들기·이름 바꾸기용 ---
in_len:         db      0
il_first:       db      0
il_pr:          dw      0
f8_s:           dw      0
need_lfn:       db      0
na_alias:       ds      11
rn_old:         ds      11
rn_hadlfn:      db      0
rn_ent:         dw      0
gn_skip:        dw      0
fdn_s:          dw      0
lb_seq:         db      0
lb_un:          db      0
lb_base:        db      0
lb_k:           db      0
lb_dst:         dw      0
lf_live:        db      0
lf_chkn:        dw      0
inbuf:          ds      104
inbuf_u:        ds      104
dl_one:         db      0
dl_all:         db      0
dl_fail:        db      0
dq_n:           db      0
dq_i:           db      0
dl_ent:         dw      0
dt_nm:          dw      0
fd_i:           dw      0
dq:             ds      32*11
lv_depth:       db      0
sm_i:           dw      0
selmap:         ds      48
cp_over:        db      0
cp_hit:         dw      0
cp_alias:       ds      11
; --- 긴 이름 읽기용 ---
lf_mode:        db      0
lf_hook:        dw      0
lf_tgt:         dw      0
lf_found:       db      0
lf_rawn:        db      0
in_found:       db      0
in_placed:      db      0
in_ia:          db      0
in_sa:          dw      0
in_ps:          dw      0
in_need:        db      0
in_sum:         db      0
rn_st:          db      0
rn_len:         db      0
pn_n:           db      0
pn_seq:         db      0
pn_chk:         db      0
er_slot:        db      0
fl_s:           dw      0
ga_dot:         dw      0
ga_bn:          db      0
ga_n:           db      0
ga_base:        ds      6
ga_ext:         ds      3
lf_orig:        ds      32
pn_list:        ds      64
ne_a:           ds      108
ne_b:           ds      108
ga_src:         ds      108
ow_all:         db      0
h_src:          db      0
h_dst:          db      0
srcpath:        ds      80
dstpath:        ds      80
sa_n:           dw      0
sa_i:           dw      0
sa_cnt:         dw      0
sa_any:         db      0
; --- 긴 이름 쓰기용 ---
ra_s:           dw      0
ra_n:           db      0
lf_panel:       dw      0
lf_end:         dw      0
lf_path:        dw      0
lf_drv:         db      0
lf_valid:       db      0
lf_done:        db      0
lf_spc:         db      0
lf_shift:       db      0
lf_fat16:       db      0
lf_fat:         dw      0
lf_root:        dw      0
lf_rsecs:       dw      0
lf_data:        dw      0
lf_clmax:       dw      0
lf_cl:          dw      0
lf_s:           dw      0
lf_sh:          dw      0                       ; 현재 섹터 번호의 윗 16 비트 (32MB 넘는 볼륨)
lf_big:         db      0                       ; 1 = 전체 섹터 수가 32 비트인 큰 볼륨
lf_ord:         db      0                       ; _RDDRV/_WRDRV 의 HL:DE 순서 (0 = 윗:아랫)
ra_hi:          dw      0
ra_f:           db      0
in_sah:         dw      0
lf_n:           dw      0
lf_n8:          db      0
lf_fcache:      dw      0
lf_exp:         db      0
lf_chk:         db      0
lf_seq:         db      0
lf_cnt:         db      0
lf_sn:          dw      0
lf_en:          dw      0
la_n:           db      0
la_dest:        dw      0
fn_cl:          dw      0
fn_off:         dw      0
en_p:           dw      0
dn_e:           dw      0
dp_i:           db      0
dp_idx:         dw      0
hl_val:         db      0
dl_key:         db      0
hbuf:           ds      80
dv_r:           db      0
dv_c:           db      0
old_linl:       db      0
old_csrsw:      db      0
old_scrmod:     db      0
old_cnsdfg:     db      0
up_p:           dw      0
up_root:        dw      0
up_last:        dw      0
sh_n:           dw      0
sh_h:           dw      0
sh_i:           dw      0
sh_j:           dw      0
sh_k:           dw      0
sh_tmp:         dw      0
sh_base:        dw      0
sh_gp:          dw      0
cmp_a:          dw      0
cmp_b:          dw      0
v32:            ds      4
d_pw:           ds      4
cnt32:          ds      4
dg_cnt:         db      0
dg_lead:        db      0
tmp10:          ds      12
nmbuf:          ds      16
upname:         ds      16
                include "s7.inc"
                include "hang.inc"
                include "ovload.inc"
                include "hanin.inc"
                include "ini.inc"
linebuf:        ds      40
oldpath:        ds      64
errbuf:         ds      72
specbuf:        ds      80
fibmem:         ds      64                      ; FIB (파일 정보 64 바이트)
pstate:         ds      PSZ*2
code_end:
                if      code_end > BSS
                jp      error_code_overlaps_bss    ; 코드가 작업 영역을 넘으면 조립 오류 (없는 이름)
                endif
