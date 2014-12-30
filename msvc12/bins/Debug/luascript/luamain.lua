

function __G__TRACKBACK__(msg)
    print("----------------------------------------")
    print("LUA ERROR: " .. tostring(errorMessage) .. "\n")
    print(debug.traceback("", 2))
    print("----------------------------------------")
end


local function main()
	print("----------------------------------------")
	print("call lua script")
	print("----------------------------------------")
end


xpcall(main, __G__TRACKBACK__)


local netio=CTestNetIO:GetNetIO()
print(netio:GetConnectIP())
