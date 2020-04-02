## About FA2Copy
FA2Copy is an extension DLL which provide some fixes for FinalAlert 2. With it, you can use FA2 more conveniently.

FA2Copy only uses hook and Windows API to achieve its functions, no memory hook.

## How to use
1. Prepare the dll. Simply, you can pull this project and compile it with X86. I had compiled one, you can just find it in the **examples** folder.
2. Import this dll using a PE Editor (like Stud_PE) as I didn't make an injector for it.
3. Modify your FinalAlert2 with exeScope or Resource Hacker to add functions. (If you need an example, there's an example in Simplified Chinese called **FA2MO.DAT** I've done for Mental Omega, you can simply modify a few things on it and it will fit your own MOD well.)
4. You can also combine this dll with **FA2Ext.dll** which is developed by AlexB, that one needs an injector to launch but really helpful.
5. You should configure 'FA2CopyData.ini' correctly, as FA2Copy uses functions like FindWindow and will get window's handle by its name, it's really necessary. Also I will provide you a sample ini file just in the **examples** folder, if something works wrong while using FA2Copy, always remember to check your configure file before reporting the bug.

## More information
I put a text file(controls.txt) in **samples** as well, you can find all controls need modifying from it.

It's a little bit complex for me to add an accelerate without MFC, now I'm working on it. But for know, FA2Copy will only use a GlobalAtom to realize the hotkey. It means you cannot use Ctrl+S/O/N with FinalAlert2 open for now. You can help to me fix or improve FA2Copy if you like and have time to do it. I'll be very happy for your help. (Maybe we can just focus on working on a map instead of editing something at the same time?)

Also, in the future I will consider to use an injector instead of modify it by hand. However, as it's not necessary as I have to modify the program anyway to add and adjust the controls, I'm not sure this will be really realized in the future.

My code style maybe not standard and have some mistake through the code, so I'd be happy to get your suggestions if it's good to improve my skills. In fact, it's my first project, after no longer working on the algorithm competition.

## Links
[Stuffhost, you can get FA2Ext from here.](http://www.stuffhost.de/ares/)

[A version migrated to FinalAlert2 by Caco, download password is 'caco'.](https://pan.baidu.com/s/1KiJFochCFo9JJpCBYCRDIg)

[The version for Mental Omega use can be easily found in my personal blog.](https://secsome.github.io/post/00002/)

[My bilibili space.](https://space.bilibili.com/143748001)