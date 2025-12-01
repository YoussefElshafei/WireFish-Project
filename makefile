# Compile to executable called wirefish
wirefish: app/main.c cli/cli.c app/app.c scanner/scanner.c tracer/tracer.c monitor/monitor.c fmt/fmt.c net/net.c model/model.h cli/cli.h app/app.h scanner/scanner.h tracer/tracer.h monitor/monitor.h fmt/fmt.h net/net.h tracer/icmp.c tracer/icmp.h timeutil/timeutil.c timeutil/timeutil.h
	gcc -o wirefish app/main.c cli/cli.c app/app.c scanner/scanner.c tracer/tracer.c monitor/monitor.c fmt/fmt.c net/net.c tracer/icmp.c timeutil/timeutil.c

# Compile to executable called wirefish-test with coverage
wirefish-test: app/main.c app/app.c cli/cli.c scanner/scanner.c tracer/tracer.c tracer/icmp.c monitor/monitor.c fmt/fmt.c net/net.c timeutil/timeutil.c
	gcc --coverage app/main.c app/app.c cli/cli.c scanner/scanner.c tracer/tracer.c tracer/icmp.c monitor/monitor.c fmt/fmt.c net/net.c timeutil/timeutil.c -o wirefish-test

