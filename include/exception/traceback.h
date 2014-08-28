/// @license MIT license http://opensource.org/licenses/MIT
/// @author David Siroky <siroky@dasir.cz>
/// @file

#pragma once

namespace xhnet
{
	// try catch c++异常，然后使用print_traceback打印内存堆栈

	// windows 下
	// 1、使用 _set_se_translator 转化VC SEH 为 c++异常
	// 2、开启 EHa 异常编译选项，
	// 选项1函数跟线程相关（每个线程都要设置），而且如果调用的模块也有此设置，那么设置函数会被替换

	// linux 下
	// 1、据说无法捕获异常
	// 2、采用signal捕获中端，中端回调函数抛出自定义异常，然后在自定义异常的构造或者析构中打印堆栈


	// 打印当前堆栈
	void print_traceback();


	// 转化异常
	//
	// windows下 每个线程都要调用下
	// 这个是程序开启Eha _set_se_translator实现，
	// 也可以hook _CallSETranslator来实现
	//
	// linux下 只需调用一次
	void open_exception_translator();


	// 开启dump
	// windows 下minidump, 也可以hook
	//
	// linux 只要编译选项有 -g ，即可开启
	//
	void open_dump();
};


