let rec fibonacci n =
    if n <= 0 then 0
    elif n = 1 then 1
    else fibonacci(n - 1) + fibonacci(n - 2)

let n = 10
printfn "Fibonacci(%d) = %d" n (fibonacci n)