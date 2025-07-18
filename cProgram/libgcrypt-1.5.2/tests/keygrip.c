/* keygrip.c - verifies that keygrips are calculated as expected
 *	Copyright (C) 2005 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "../src/gcrypt.h"

static int verbose;
static int repetitions;



static void
die (const char *format, ...)
{
  va_list arg_ptr;

  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  va_end (arg_ptr);
  exit (1);
}

static void
print_hex (const char *text, const void *buf, size_t n)
{
  const unsigned char *p = buf;

  fputs (text, stdout);
  for (; n; n--, p++)
    printf ("%02X", *p);
  putchar ('\n');
}




static struct
{
  int algo;
  const char *key;
  const unsigned char grip[20];
} key_grips[] =
  {
    {
      GCRY_PK_RSA,
      "(private-key"
      " (rsa"
      "  (n #00B6B509596A9ECABC939212F891E656A626BA07DA8521A9CAD4C08E640C04052FBB87F424EF1A0275A48A9299AC9DB69ABE3D0124E6C756B1F7DFB9B842D6251AEA6EE85390495CADA73D671537FCE5850A932F32BAB60AB1AC1F852C1F83C625E7A7D70CDA9EF16D5C8E47739D77DF59261ABE8454807FF441E143FBD37F8545#)"
      "  (e #010001#)"
      "  (d #077AD3DE284245F4806A1B82B79E616FBDE821C82D691A65665E57B5FAD3F34E67F401E7BD2E28699E89D9C496CF821945AE83AC7A1231176A196BA6027E77D85789055D50404A7A2A95B1512F91F190BBAEF730ED550D227D512F89C0CDB31AC06FA9A19503DDF6B66D0B42B9691BFD6140EC1720FFC48AE00C34796DC899E5#)"
      "  (p #00D586C78E5F1B4BF2E7CD7A04CA091911706F19788B93E44EE20AAF462E8363E98A72253ED845CCBF2481BB351E8557C85BCFFF0DABDBFF8E26A79A0938096F27#)"
      "  (q #00DB0CDF60F26F2A296C88D6BF9F8E5BE45C0DDD713C96CC73EBCB48B061740943F21D2A93D6E42A7211E7F02A95DCED6C390A67AD21ECF739AE8A0CA46FF2EBB3#)"
      "  (u #33149195F16912DB20A48D020DBC3B9E3881B39D722BF79378F6340F43148A6E9FC5F53E2853B7387BA4443BA53A52FCA8173DE6E85B42F9783D4A7817D0680B#)))",
      "\x32\xCF\xFA\x85\xB1\x79\x1F\xBB\x26\x14\xE9\x1A\xFD\xF3\xAF\xE3\x32\x08\x2E\x25"
    },
    {
      GCRY_PK_DSA,
      " (public-key"
      " (dsa"
      "  (p #0084E4C626E16005770BD9509ABF7354492E85B8C0060EFAAAEC617F725B592FAA59DF5460575F41022776A9718CE62EDD542AB73C7720869EBDBC834D174ADCD7136827DF51E2613545A25CA573BC502A61B809000B6E35F5EB7FD6F18C35678C23EA1C3638FB9CFDBA2800EE1B62F41A4479DE824F2834666FBF8DC5B53C2617#)"
      "  (q #00B0E6F710051002A9F425D98A677B18E0E5B038AB#)"
      "  (g #44370CEE0FE8609994183DBFEBA7EEA97D466838BCF65EFF506E35616DA93FA4E572A2F08886B74977BC00CA8CD3DBEA7AEB7DB8CBB180E6975E0D2CA76E023E6DE9F8CCD8826EBA2F72B8516532F6001DEFFAE76AA5E59E0FA33DBA3999B4E92D1703098CDEDCC416CF008801964084CDE1980132B2B78CB4CE9C15A559528B#)"
      "  (y #3D5DD14AFA2BF24A791E285B90232213D0E3BA74AB1109E768AED19639A322F84BB7D959E2BA92EF73DE4C7F381AA9F4053CFA3CD4527EF9043E304E5B95ED0A3A5A9D590AA641C13DB2B6E32B9B964A6A2C730DD3EA7C8E13F7A140AFF1A91CE375E9B9B960384779DC4EA180FA1F827C52288F366C0770A220F50D6D8FD6F6#)))",
      "\x04\xA3\x4F\xA0\x2B\x03\x94\xD7\x32\xAD\xD5\x9B\x50\xAF\xDB\x5D\x57\x22\xA6\x10"

    },
    {
      GCRY_PK_DSA,
      "(private-key"
      " (dsa"
      "  (p #0084E4C626E16005770BD9509ABF7354492E85B8C0060EFAAAEC617F725B592FAA59DF5460575F41022776A9718CE62EDD542AB73C7720869EBDBC834D174ADCD7136827DF51E2613545A25CA573BC502A61B809000B6E35F5EB7FD6F18C35678C23EA1C3638FB9CFDBA2800EE1B62F41A4479DE824F2834666FBF8DC5B53C2617#)"
      "  (q #00B0E6F710051002A9F425D98A677B18E0E5B038AB#)"
      "  (g #44370CEE0FE8609994183DBFEBA7EEA97D466838BCF65EFF506E35616DA93FA4E572A2F08886B74977BC00CA8CD3DBEA7AEB7DB8CBB180E6975E0D2CA76E023E6DE9F8CCD8826EBA2F72B8516532F6001DEFFAE76AA5E59E0FA33DBA3999B4E92D1703098CDEDCC416CF008801964084CDE1980132B2B78CB4CE9C15A559528B#)"
      "  (y #3D5DD14AFA2BF24A791E285B90232213D0E3BA74AB1109E768AED19639A322F84BB7D959E2BA92EF73DE4C7F381AA9F4053CFA3CD4527EF9043E304E5B95ED0A3A5A9D590AA641C13DB2B6E32B9B964A6A2C730DD3EA7C8E13F7A140AFF1A91CE375E9B9B960384779DC4EA180FA1F827C52288F366C0770A220F50D6D8FD6F6#)"
      "  (x #0087F9E91BFBCC1163DE71ED86D557708E32F8ADDE#)))",
      "\x04\xA3\x4F\xA0\x2B\x03\x94\xD7\x32\xAD\xD5\x9B\x50\xAF\xDB\x5D\x57\x22\xA6\x10"
    },
    {
      GCRY_PK_ECDSA,
      "(public-key"
      " (ecdsa"
      " (p #00FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF#)"
      " (a #00FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC#)"
      " (b #5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B#)"
      " (g #046B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C2964FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5#)"
      " (n #00FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551#)"
      " (q #04C8A4CEC2E9A9BC8E173531A67B0840DF345C32E261ADD780E6D83D56EFADFD5DE872F8B854819B59543CE0B7F822330464FBC4E6324DADDCD9D059554F63B344#)))",
      "\xE6\xDF\x94\x2D\xBD\x8C\x77\x05\xA3\xDD\x41\x6E\xFC\x04\x01\xDB\x31\x0E\x99\xB6"
    },
    {
      GCRY_PK_ECDSA,
      "(public-key"
      " (ecdsa"
      " (p #00FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF#)"
      " (curve \"NIST P-256\")"
      " (b #5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B#)"
      " (g #046B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C2964FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5#)"
      " (n #00FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551#)"
      " (q #04C8A4CEC2E9A9BC8E173531A67B0840DF345C32E261ADD780E6D83D56EFADFD5DE872F8B854819B59543CE0B7F822330464FBC4E6324DADDCD9D059554F63B344#)))",
      "\xE6\xDF\x94\x2D\xBD\x8C\x77\x05\xA3\xDD\x41\x6E\xFC\x04\x01\xDB\x31\x0E\x99\xB6"
    },
    {
      GCRY_PK_ECDSA,
      "(public-key"
      " (ecdsa"
      " (curve secp256r1)"
      " (q #04C8A4CEC2E9A9BC8E173531A67B0840DF345C32E261ADD780E6D83D56EFADFD5DE872F8B854819B59543CE0B7F822330464FBC4E6324DADDCD9D059554F63B344#)))",
      "\xE6\xDF\x94\x2D\xBD\x8C\x77\x05\xA3\xDD\x41\x6E\xFC\x04\x01\xDB\x31\x0E\x99\xB6"
    }

  };

static void
check (void)
{
  unsigned char buf[20];
  unsigned char *ret;
  gcry_error_t err;
  gcry_sexp_t sexp;
  unsigned int i;
  int repn;

  for (i = 0; i < (sizeof (key_grips) / sizeof (*key_grips)); i++)
    {
      if (gcry_pk_test_algo (key_grips[i].algo))
        {
          if (verbose)
            fprintf (stderr, "algo %d not available; test skipped\n",
                     key_grips[i].algo);
          continue;
        }
      err = gcry_sexp_sscan (&sexp, NULL, key_grips[i].key,
			     strlen (key_grips[i].key));
      if (err)
        die ("scanning data %d failed: %s\n", i, gpg_strerror (err));

      for (repn=0; repn < repetitions; repn++)
        {
          ret = gcry_pk_get_keygrip (sexp, buf);
          if (!ret)
            die ("gcry_pk_get_keygrip failed for %d\n", i);

          if ( memcmp (key_grips[i].grip, buf, sizeof (buf)) )
            {
              print_hex ("keygrip: ", buf, sizeof buf);
              die ("keygrip for %d does not match\n", i);
            }
        }

      gcry_sexp_release (sexp);
    }
}



static void
progress_handler (void *cb_data, const char *what, int printchar,
		  int current, int total)
{
  (void)cb_data;
  (void)what;
  (void)current;
  (void)total;

  putchar (printchar);
}

int
main (int argc, char **argv)
{
  int last_argc = -1;
  int debug = 0;

  if (argc)
    { argc--; argv++; }

  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--"))
        {
          argc--; argv++;
          break;
        }
      else if (!strcmp (*argv, "--verbose"))
        {
          verbose = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose = 1;
          debug = 1;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--repetitions"))
        {
          argc--; argv++;
          if (argc)
            {
              repetitions = atoi(*argv);
              argc--; argv++;
            }
        }
    }

  if (repetitions < 1)
    repetitions = 1;

  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  gcry_set_progress_handler (progress_handler, NULL);

  gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
  if (debug)
    gcry_control (GCRYCTL_SET_DEBUG_FLAGS, 1u, 0);

  check ();

  return 0;
}
