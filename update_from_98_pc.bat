chcp 65001
net use Z: "\\gear99\C_GEAR99" /persistent:no
rem set netdisk=Z
Z:
cd "Мои документы\Flying_Cards_dx5_v3"
copy *.* "C:\Users\sergey\Documents\GitHub\flying_cards"
net use Z: /d /y
pause