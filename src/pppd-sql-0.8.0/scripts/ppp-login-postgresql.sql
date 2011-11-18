--
-- PostgreSQL database dump
--

SET client_encoding = 'UTF8';
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- Name: ppp; Type: DATABASE; Schema: -; Owner: postgres
--

CREATE DATABASE ppp WITH TEMPLATE = template0 ENCODING = 'UTF8';


ALTER DATABASE ppp OWNER TO postgres;

\connect ppp

SET client_encoding = 'UTF8';
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- Name: SCHEMA public; Type: COMMENT; Schema: -; Owner: postgres
--

COMMENT ON SCHEMA public IS 'Standard public schema';


SET search_path = public, pg_catalog;

--
-- Name: sq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE sq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


ALTER TABLE public.sq OWNER TO postgres;

--
-- Name: sq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('sq', 1, false);


SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: login; Type: TABLE; Schema: public; Owner: postgres; Tablespace: 
--

CREATE TABLE "login" (
    id integer DEFAULT nextval('sq'::regclass) NOT NULL,
    username character varying(16) NOT NULL,
    "password" character varying(32) NOT NULL,
    status integer DEFAULT 0 NOT NULL,
    clientip character varying(15) NOT NULL,
    serverip character varying(15) NOT NULL
);


ALTER TABLE public."login" OWNER TO postgres;

--
-- Data for Name: login; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY "login" (id, username, "password", status, clientip, serverip) FROM stdin;
\.


--
-- Name: public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

