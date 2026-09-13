// web_page.h

#ifndef _WEB_PAGE_H_
#define _WEB_PAGE_H_

static const char gstrWebPage[] =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'>"
"<title>Switch 2 Controller</title>"
"<style>"
"body{font-family:Arial;background:#111;color:#fff;text-align:center;margin:0;padding:20px;}"
"h1{font-size:24px;margin-bottom:20px;}"
".row{display:flex;justify-content:center;gap:12px;margin:12px 0;}"
".btn{width:72px;height:72px;border:none;border-radius:50%;font-size:22px;font-weight:bold;background:#333;color:#fff;}"
".btn:active{background:#666;}"
".wide{width:100px;border-radius:16px;}"
".small{width:64px;height:48px;border-radius:12px;font-size:16px;}"
".dpad{display:grid;grid-template-columns:70px 70px 70px;grid-template-rows:70px 70px 70px;gap:4px;justify-content:center;margin:20px auto;}"
".dpad button{width:70px;height:70px;border:none;border-radius:12px;background:#333;color:#fff;font-size:28px;}"
".blank{visibility:hidden;}"
"</style>"
"</head>"
"<body>"

"<h1>Switch 2 Controller</h1>"

"<div class='row'>"
"<button class='btn small' data-key='zl'>ZL</button>"
"<button class='btn small' data-key='l'>L</button>"
"<button class='btn small' data-key='r'>R</button>"
"<button class='btn small' data-key='zr'>ZR</button>"
"</div>"

"<div class='row'>"
"<button class='btn small' data-key='minus'>-</button>"
"<button class='btn small' data-key='capture'>CAP</button>"
"<button class='btn small' data-key='home'>HOME</button>"
"<button class='btn small' data-key='plus'>+</button>"
"</div>"

"<div class='dpad'>"
"<button class='blank'></button>"
"<button data-key='up'>↑</button>"
"<button class='blank'></button>"

"<button data-key='left'>←</button>"
"<button class='blank'></button>"
"<button data-key='right'>→</button>"

"<button class='blank'></button>"
"<button data-key='down'>↓</button>"
"<button class='blank'></button>"
"</div>"

"<div class='row'>"
"<button class='btn' data-key='y'>Y</button>"
"<button class='btn' data-key='x'>X</button>"
"</div>"

"<div class='row'>"
"<button class='btn' data-key='b'>B</button>"
"<button class='btn' data-key='a'>A</button>"
"</div>"

"<div class='row'>"
"<button class='btn small' data-key='gl'>GL</button>"
"<button class='btn small' data-key='c'>C</button>"
"<button class='btn small' data-key='gr'>GR</button>"
"</div>"

"<script>"
"function send(key,state){"
"fetch('/api/'+key+'/'+state,{cache:'no-store'}).catch(()=>{});"
"}"

"document.querySelectorAll('[data-key]').forEach(function(btn){"
"const key=btn.dataset.key;"

"btn.addEventListener('mousedown',function(e){"
"e.preventDefault();"
"send(key,'down');"
"});"

"btn.addEventListener('mouseup',function(e){"
"e.preventDefault();"
"send(key,'up');"
"});"

"btn.addEventListener('mouseleave',function(e){"
"send(key,'up');"
"});"

"btn.addEventListener('touchstart',function(e){"
"e.preventDefault();"
"send(key,'down');"
"},{passive:false});"

"btn.addEventListener('touchend',function(e){"
"e.preventDefault();"
"send(key,'up');"
"},{passive:false});"

"btn.addEventListener('touchcancel',function(e){"
"send(key,'up');"
"});"
"});"
"</script>"

"</body>"
"</html>";

#endif