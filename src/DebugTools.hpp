# pragma once
# define DEBUG_PART(x) x
# define INFO(x) DEBUG_PART(std::cerr << "[INFO|" << __func__ << "] " <<  x << std::endl)
# define snap(var) DEBUG_PART(std::cerr << "[snap] " << #var << " = " << (var) << std::endl)
# define snap_msg(msg, var) DEBUG_PART(std::cerr << "[snap:" << (msg) << "] " << #var << " = " << (var) << std::endl)
