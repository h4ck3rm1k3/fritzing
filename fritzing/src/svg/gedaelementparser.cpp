// This file was generated by qlalr - DO NOT EDIT!

/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision: 1671 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-28 12:20:31 +0100 (Fri, 28 Nov 2008) $

********************************************************************/

#include <QDebug>
#include "gedaelementparser.h"
#include "gedaelementlexer.h"

GedaElementParser::GedaElementParser()
{
}

GedaElementParser::~GedaElementParser()
{
}

QVector<QVariant> & GedaElementParser::symStack() {
	return m_symStack;
}

void GedaElementParser::reallocateStack()
{
    int size = m_stateStack.size();
    if (size == 0)
        size = 128;
    else
        size <<= 1;

    m_stateStack.resize(size);
}

QString GedaElementParser::errorMessage() const
{
    return m_errorMessage;
}

QVariant GedaElementParser::result() const
{
    return m_result;
}

bool GedaElementParser::parse(GedaElementLexer *lexer)
{
  const int INITIAL_STATE = 0;

  int yytoken = -1;

  reallocateStack();

  m_tos = 0;
  m_stateStack[++m_tos] = INITIAL_STATE;

  while (true) {
      const int state = m_stateStack.at(m_tos);
      if (yytoken == -1 && - TERMINAL_COUNT != action_index [state])
        yytoken = lexer->lex();
      int act = t_action (state, yytoken);
      if (act == ACCEPT_STATE)
        return true;

      else if (act > 0) {
          if (++m_tos == m_stateStack.size())
            reallocateStack();
          m_stateStack[m_tos] = act;
          yytoken = -1;
      } else if (act < 0) {
          int r = - act - 1;

          m_tos -= rhs [r];
          act = m_stateStack.at(m_tos++);

          switch (r) {
 case 0: {
    qDebug() << "got geda_element ";
} break;  case 1: {
    qDebug() << "    got element_arguments ";
} break;  case 3: {
    qDebug() << "    got sub_element_groups ";
} break;  case 7: {
    qDebug() << "    got sub_element_group ";
} break;  case 8: {
    qDebug() << "got pin_element ";
} break;  case 10: {
    qDebug() << "    got pin_sequence ";
} break;  case 11: {
	m_symStack.append(")");
    qDebug() << "    got pin_paren_sequence ";
} break;  case 12: {
	m_symStack.append("]");
    qDebug() << "    got pin_bracket_sequence ";
} break;  case 13: {
    qDebug() << "    got pin_arguments ";
} break;  case 14: {
    qDebug() << "got pad_element ";
} break;  case 16: {
    qDebug() << "    got pad_sequence ";
} break;  case 17: {
	m_symStack.append(")");
    qDebug() << "    got pad_paren_sequence ";
} break;  case 18: {
	m_symStack.append("]");
    qDebug() << "    got pad_bracket_sequence ";
} break;  case 19: {
    qDebug() << "    got pad_arguments ";
} break;  case 20: {
    qDebug() << "got element_line_element ";
} break;  case 22: {
    qDebug() << "    got element_line_sequence ";
} break;  case 23: {
	m_symStack.append(")");
    qDebug() << "    got element_line_paren_sequence ";
} break;  case 24: {
	m_symStack.append("]");
    qDebug() << "    got element_line_bracket_sequence ";
} break;  case 25: {
    qDebug() << "    got element_line_arguments ";
} break;  case 26: {
    qDebug() << "got element_arc_element ";
} break;  case 28: {
    qDebug() << "    got element_arc_sequence ";
} break;  case 29: {
	m_symStack.append(")");
    qDebug() << "    got element_arc_paren_sequence ";
} break;  case 30: {
	m_symStack.append("]");
    qDebug() << "    got element_arc_bracket_sequence ";
} break;  case 31: {
    qDebug() << "    got element_arc_arguments ";
} break;  case 32: {
} break;  case 33: {
} break;  case 34: {
} break;  case 35: {
} break;  case 36: {
} break;  case 37: {
} break;  case 38: {
} break;  case 39: {
} break;  case 40: {
} break;  case 41: {
} break;  case 42: {
} break;  case 43: {
} break;  case 44: {
} break;  case 45: {
} break;  case 46: {
} break;  case 47: {
} break;  case 48: {
} break;  case 49: {
} break;  case 50: {
} break;  case 51: {
} break;  case 52: {
} break;  case 53: {
} break;  case 54: {
} break;  case 55: {
} break;  case 56: {
} break;  case 57: {
} break;  case 58: {
} break;  case 59: {
} break;  case 60: {
} break;  
case 61: {
    qDebug() << "        got NUMBER ";
    m_symStack.append(lexer->currentNumber());
} break; 
 
case 62: {
    qDebug() << "        got STRING ";
    m_symStack.append(lexer->currentString());
} break; 
 
case 63: {
    qDebug() << "got ELEMENT command ";
    m_symStack.append(lexer->currentCommand());
} break; 
 
case 64: {
    qDebug() << "got PIN command ";
    m_symStack.append(lexer->currentCommand());
} break; 
 
case 65: {
    qDebug() << "got PAD command ";
    m_symStack.append(lexer->currentCommand());
} break; 
 
case 66: {
    qDebug() << "got ELEMENTLINE command ";
    m_symStack.append(lexer->currentCommand());
} break; 
 
case 67: {
    qDebug() << "got ELEMENTARC command ";
    m_symStack.append(lexer->currentCommand());
} break; 

          } // switch

          m_stateStack[m_tos] = nt_action(act, lhs[r] - TERMINAL_COUNT);

      } else {
          int ers = state;
          int shifts = 0;
          int reduces = 0;
          int expected_tokens[3];
          for (int tk = 0; tk < TERMINAL_COUNT; ++tk) {
              int k = t_action(ers, tk);

              if (! k)
                continue;
              else if (k < 0)
                ++reduces;
              else if (spell[tk]) {
                  if (shifts < 3)
                    expected_tokens[shifts] = tk;
                  ++shifts;
              }
          }

          m_errorMessage.clear();
          if (shifts && shifts < 3) {
              bool first = true;

              for (int s = 0; s < shifts; ++s) {
                  if (first)
                    m_errorMessage += QLatin1String("Expected ");
                  else
                    m_errorMessage += QLatin1String(", ");

                  first = false;
                  m_errorMessage += QLatin1String("`");
                  m_errorMessage += QLatin1String(spell[expected_tokens[s]]);
                  m_errorMessage += QLatin1String("'");
              }
          }

          return false;
        }
    }

    return false;
}
