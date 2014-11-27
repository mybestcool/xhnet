
//#define SMART_ASSERT_DEBUG_MODE	1
#include "exception/smart_assert.h"

#include <sstream>
#include "ensure.h"
#include "xhlog.h"

namespace xhnet
{
	void ensure_log(const assert_context & context)
	{
		std::ostringstream out;

		out << smart_assert::get_typeof_level(context.get_level())
			<< " in " << context.get_context_file() << ":" << context.get_context_line() << '\n';
		if (!context.get_level_msg().empty())
			out << "User-friendly msg: '" << context.get_level_msg() << "'\n";
		out << "Expression: '" << context.get_expr() << "'\n";

		typedef assert_context::vals_array vals_array;
		const vals_array & aVals = context.get_vals_array();
		if (!aVals.empty()) {
			bool bFirstTime = true;
			vals_array::const_iterator first = aVals.begin(), last = aVals.end();
			while (first != last) {
				if (bFirstTime) {
					out << "Values: ";
					bFirstTime = false;
				}
				else {
					out << "        ";
				}
				out << first->second << "='" << first->first << "'\n";
				++first;
			}
		}
		
		XH_LOG_INFO(::xhnet::logname_trace, out.str());
	}

	void ensure_handle(const assert_context & context)
	{
		//什么都不做
	}

	static CInsureInit sf_init_insure;

	CInsureInit::CInsureInit()
	{
		Assert::set_log(&::xhnet::ensure_log);
		Assert::set_handler(lvl_warn, &::xhnet::ensure_handle);
		Assert::set_handler(lvl_debug, &::xhnet::ensure_handle);
		Assert::set_handler(lvl_error, &::xhnet::ensure_handle);
		Assert::set_handler(lvl_fatal, &::xhnet::ensure_handle);
	}

	CInsureInit::~CInsureInit()
	{

	}
};
