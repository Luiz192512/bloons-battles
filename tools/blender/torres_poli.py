"""As torres do piloto (dardo, bomba e bucaneiro) na variante base.

As pecas moram em torres/<chave>.py e saem do montador (partes.montar); este modulo so mantem a
entrada antiga do gerar_poli.py, que grava o modelo unico e as folhas de revisao do piloto.
"""
import partes


def _base(chave):
    return lambda c: partes.montar(c, chave, 0, 0, 0)


TORRES = {chave: _base(chave) for chave in ("dardo", "bomba", "bucaneiro")}
# alvo e tamanho do enquadramento das folhas de revisao
ENQUADRE = {"dardo": ((0, 0.03, 0.52), 1.25), "bomba": ((0, -0.02, 0.33), 1.15), "bucaneiro": ((0, -0.05, 0.68), 2.5)}
