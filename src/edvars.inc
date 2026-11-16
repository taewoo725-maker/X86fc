; 만든이: KimTaeWoo
; X86FC 편집기 상태 변수 (오버레이 1·10 이 같이 씀) - SPDX-License-Identifier: GPL-3.0-or-later
; 쪽 3 의 LVSTK(폴더 복사 스택·보기 줄 버퍼와 같은 자리: 편집기가 도는 동안 다른 이는 안 씀)에 둔다.  오버레이가 바뀌어도 남아야 하므로.
EV              equ     LVSTK
v_name          equ     EV+0                    ; 파일 이름 (제목 줄, 16)
v_row           equ     EV+16
eh_was          equ     EV+17
eh_key          equ     EV+18
eh_cp           equ     EV+19                   ; 2
eh_n            equ     EV+21
eh_p            equ     EV+22                   ; 2
ed_rows         equ     EV+24
ed_gt           equ     EV+25
rb_c            equ     EV+26
rb_nb           equ     EV+27
rb_g            equ     EV+28                   ; 2
mp_all          equ     EV+30                   ; 2
mp_fre          equ     EV+32                   ; 2
mp_put          equ     EV+34                   ; 2
mp_get          equ     EV+36                   ; 2
ed_end          equ     EV+38                   ; 2
ed_cur          equ     EV+40                   ; 2
ed_top          equ     EV+42                   ; 2
ed_sz           equ     EV+44                   ; 2
ed_wn           equ     EV+46                   ; 2
ed_t1           equ     EV+48                   ; 2
pv_e            equ     EV+50                   ; 2
pv_s0           equ     EV+52                   ; 2
ed_mod          equ     EV+54
ed_seg          equ     EV+55
ed_old          equ     EV+56
ed_h            equ     EV+57
ed_row          equ     EV+58
ed_c1           equ     EV+59
ed_cc           equ     EV+60
ed_attr         equ     EV+61
ed_ch           equ     EV+62
ed_t2           equ     EV+63                   ; 2
; (0.12.42) 명령 오버레이(10)
ed_cmd          equ     EV+65                   ; 1 찾기 2 다음 3 바꾸기 4 줄 이동 5 이것 바꾸고 다음 6 전부 7 건너뜀
ed_rp           equ     EV+66                   ; 1 = 바꾸기 확인 중
ln_p            equ     EV+67                   ; 줄 번호를 센 자리 (0 = 없음)
ln_n            equ     EV+69                   ; 그 자리의 줄 번호
ed_t3           equ     EV+71                   ; 2
ed_t4           equ     EV+73                   ; 2
ed_cnt          equ     EV+75                   ; 2
; 찾는 글(FS)·바꿀 글(RS): +0 입력 글 길이, +1 입력 글(inbuf 형식, 48), +49 UTF-8 길이, +50 UTF-8 글 (49)
FS              equ     EV+77
RS              equ     EV+176
fs_n            equ     FS+49
fs_str          equ     FS+50
rs_n            equ     RS+49
rs_str          equ     RS+50
; (0.12.43) 블록 선택·복사
sel_a           equ     EV+275                  ; 2 선택 시작 자리 (0 = 없음).  다른 쪽 끝 = 커서
sel_m           equ     EV+277                  ; 1 = Ctrl+B 로 시작(이동해도 안 풀림)
sel_lo          equ     EV+278                  ; 2 그리기용 (작은 쪽)
sel_hi          equ     EV+280                  ; 2 (큰 쪽, 0 = 선택 없음)
hs_c0           equ     EV+282                  ; 줄 그릴 때 선택된 첫 칸 (FFh = 없음)
hs_c1           equ     EV+283                  ; 마지막 칸 + 1
clip_n          equ     EV+284                  ; 2 클립보드 바이트 수 (글은 CPBUF)
sp_pos          equ     EV+286                  ; 2 끼워 넣기·지우기 자리
sp_dl           equ     EV+288                  ; 2 지울 길이
sp_src          equ     EV+290                  ; 2 끼울 글 자리
sp_sl           equ     EV+292                  ; 2 끼울 길이
ud_top          equ     EV+294                  ; 2 되돌리기 기록의 맨 위 (0 = 비어 있음)
ud_off          equ     EV+296                  ; 1 = 기록 안 함 (되돌리는 중·전부 바꾸기)
; (0.12.45) 큰 파일 (창 방식): 글 버퍼에는 파일의 한 토막(창)만 읽어 두고, 커서가 창 끝에 다가가면 고쳤을 때 파일에 되쓴 뒤 다음 창을 읽음
big             equ     EV+297                  ; 1 = 큰 파일 방식
bw_dn           equ     EV+298                  ; 1 = 창 뒤에 파일이 더 있음
bw_up           equ     EV+299                  ; 1 = 창 앞에 파일이 더 있음
bw_al           equ     EV+300                  ; 1 = 읽을 때 처음의 부분 줄을 건너뜀
bw_h2           equ     EV+301                  ; 두 번째 파일 핸들
win_off         equ     EV+302                  ; 4 창 시작의 파일 위치
fsz             equ     EV+306                  ; 4 디스크 위의 파일 크기
bw_off          equ     EV+310                  ; 4 읽을 위치
bw_t            equ     EV+314                  ; 4 임시
bw_c            equ     EV+318                  ; 4 임시 (커서의 파일 위치 등)
bw_p            equ     EV+322                  ; 4 임시 (옛 창 시작)
win_dl          equ     EV+326                  ; 2 디스크 위 창 길이
win_ln          equ     EV+328                  ; 2 창 첫 줄의 줄 번호
bw_n            equ     EV+330                  ; 2
bw_k            equ     EV+332                  ; 2
bw_d            equ     EV+334                  ; 2 커서 - 화면 맨 위
BUFC            equ     EV+336                  ; 복사 완충 256
; (0.12.46) EUC-KR 변환
ed_euc          equ     EV+592                  ; 1 = EUC-KR 글을 UTF-8 로 바꿔 열었음 (저장 때 EUC-KR 로 되돌림)
eu_len          equ     EV+593                  ; 2
eu_src          equ     EV+595                  ; 2
eu_c            equ     EV+597
eu_mode         equ     EV+598                  ; 0 = 셈, 1 = 씀
eu_o            equ     EV+599                  ; 완충에 담은 바이트 수
; (0.12.50) 클립보드를 따로 확보한 매퍼 쪽(clip_seg)에 둠 (없으면 CPBUF 760바이트)
clip_seg        equ     EV+600                  ; 0 = 별도 쪽 없음
cl_a            equ     EV+601                  ; 2
cl_o            equ     EV+603                  ; 2
cl_c            equ     EV+605                  ; 2
EVN             equ     607
LOADSZ          equ     12288
KEEP            equ     6000
; (0.12.44) 글 버퍼 = 쪽 하나의 앞쪽 EDMAX 바이트, 뒤쪽 LOGSZ 바이트 = 되돌리기 기록 (같은 쪽이라 글을 끼운 동안 항상 보임)
EDBUF           equ     8000h
EDMAX           equ     14848
LOG             equ     EDBUF+EDMAX
LOGSZ           equ     1536
CLIP            equ     CPBUF                   ; 클립보드 (CLIPMAX 바이트).  CPBUF 는 압축 목록·복사 때만 쓰므로 편집 중에는 비어 있음
CLIPMAX         equ     760
IMG             equ     CPBUF+768               ; SCREEN 0 깜박임 표 그림 (240)
