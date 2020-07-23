from setuptools import setup, find_packages

setup (
  name='scopeslexer',
  packages=find_packages(),
  entry_points =
  """
  [pygments.lexers]
  scopeslexer = scopeslexer.lexer:ScopesLexer
  """,
)
