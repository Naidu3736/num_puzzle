def generar_archivo_puzzle(dim, nombre_archivo):
    mat = [[0] * dim for _ in range(dim)]
    cr, cc = dim // 2, (dim - 1) // 2
    
    val = 1
    t, b, l, r = 0, dim - 1, 0, dim - 1
    while t <= b and l <= r:
        for i in range(l, r + 1):
            if not (t == cr and i == cc):
                mat[t][i] = val
                val += 1
        t += 1
        for i in range(t, b + 1):
            if not (i == cr and r == cc):
                mat[i][r] = val
                val += 1
        r -= 1
        if t <= b:
            for i in range(r, l - 1, -1):
                if not (b == cr and i == cc):
                    mat[b][i] = val
                    val += 1
            b -= 1
        if l <= r:
            for i in range(b, t - 1, -1):
                if not (i == cr and l == cc):
                    mat[i][l] = val
                    val += 1
            l += 1
    mat[cr][cc] = 0
    
    # Como estado inicial usamos el mismo tablero (o puedes desordenarlo levemente)
    # Escribir archivo de entrada para el solver
    with open(nombre_archivo, "w") as f:
        f.write(f"{dim}\n")
        # Estado inicial (ejemplo idéntico o base)
        for row in mat:
            f.write(",".join(map(str, row)) + "\n")
        # Estado meta (en espiral con 0 al centro)
        for row in mat:
            f.write(",".join(map(str, row)) + "\n")


while True:
    n = int(input("dimensio: "))
    generar_archivo_puzzle(n, f"examples/{n}x{n}.txt")
    print("¡Archivo de prueba generado con éxito!")