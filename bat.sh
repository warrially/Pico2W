req() {
    if curl -fsS -o /dev/null "$1"; then
        echo "操作成功: $1"
    else
        echo "操作失败: $1"
    fi
}

    req "http://192.168.1.104/api/lstick/set?x=128&y=128"
    req http://192.168.1.104/api/zr/up


    req http://192.168.1.104/api/a/click
    sleep 1
    req http://192.168.1.104/api/a/click
    sleep 1


while true; do
    req http://192.168.1.104/api/a/click
    sleep 1
    req http://192.168.1.104/api/a/click
    sleep 1
    req http://192.168.1.104/api/a/click
    sleep 1
    req http://192.168.1.104/api/a/click
    sleep 1
    req http://192.168.1.104/api/a/click
    sleep 1
    req http://192.168.1.104/api/a/click
    sleep 1
    req http://192.168.1.104/api/a/click
    sleep 1


    req http://192.168.1.104/api/x/click
    sleep 1

    req http://192.168.1.104/api/a/click
    sleep 1

    req http://192.168.1.104/api/a/click
    sleep 1

    req http://192.168.1.104/api/a/click

    sleep 5

    req http://192.168.1.104/api/zr/down
    req http://192.168.1.104/api/zr/up
    req http://192.168.1.104/api/zr/down
    sleep 30

    req http://192.168.1.104/api/rstick/click
    sleep 1
    req http://192.168.1.104/api/b/click
    sleep 1
    req http://192.168.1.104/api/rstick/click
    sleep 1
    req http://192.168.1.104/api/b/click
    sleep 1
    req http://192.168.1.104/api/rstick/click
    sleep 1
    req http://192.168.1.104/api/b/click
    sleep 1
    req http://192.168.1.104/api/rstick/click
    sleep 1
    req http://192.168.1.104/api/b/click
    sleep 1
    req http://192.168.1.104/api/rstick/click
    sleep 1
    req http://192.168.1.104/api/b/click
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=0"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=128"
    sleep 1
    req http://192.168.1.104/api/rstick/click
    sleep 1
    req http://192.168.1.104/api/b/click
    sleep 1
    req http://192.168.1.104/api/rstick/click
    sleep 1
    req http://192.168.1.104/api/b/click
    sleep 1
    req http://192.168.1.104/api/rstick/click
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=0"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=128"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=0"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=128"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=0"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=128"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=0"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=128"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=0"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=128"
    sleep 1


    # 左两步
    req "http://192.168.1.104/api/lstick/set?x=128&y=0"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=0&y=128"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=256&y=128"
    sleep 2
    req "http://192.168.1.104/api/lstick/set?x=0&y=128"
    sleep 1

    # 左两步
    req "http://192.168.1.104/api/lstick/set?x=128&y=0"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=0&y=128"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=256&y=128"
    sleep 2
    req "http://192.168.1.104/api/lstick/set?x=0&y=128"
    sleep 1

    # 左两步
    req "http://192.168.1.104/api/lstick/set?x=128&y=0"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=0&y=128"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=256&y=128"
    sleep 2
    req "http://192.168.1.104/api/lstick/set?x=0&y=128"
    sleep 1

    # 左两步
    req "http://192.168.1.104/api/lstick/set?x=128&y=0"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=0&y=128"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=256&y=128"
    sleep 2
    req "http://192.168.1.104/api/lstick/set?x=0&y=128"
    sleep 1
    req "http://192.168.1.104/api/lstick/set?x=128&y=128"

    # 强退兜底按键
    req "http://192.168.1.104/api/plus/click"
    sleep 1
    req "http://192.168.1.104/api/down/click"
    sleep 1
    req "http://192.168.1.104/api/a/click"
    sleep 1
    req "http://192.168.1.104/api/right/click"
    sleep 1
    req "http://192.168.1.104/api/a/click"
    sleep 1
    req http://192.168.1.104/api/zr/up
done
